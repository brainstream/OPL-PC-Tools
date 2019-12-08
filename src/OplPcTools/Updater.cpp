/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
 *                                                                                             *
 * This file is part of the OPL PC Tools project, the graphical PC tools for Open PS2 Loader.  *
 *                                                                                             *
 * OPL PC Tools is free software: you can redistribute it and/or modify it under the terms of  *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * OPL PC Tools is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <QVector>
#include <QRegExp>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSysInfo>
#include <QCoreApplication>
#include <QFileInfo>
#include <OplPcTools/ApplicationInfo.h>
#include <OplPcTools/Updater.h>

using namespace OplPcTools;

namespace {

enum class Platform
{
    Linux,
    Windows
};

Maybe<Platform> buildPlatform()
{
#if defined(__linux__)
    return Platform::Linux;
#elif defined(_WIN32)
    return Platform::Windows;
#else
    return nullptr;
#endif
}

enum class Architecture
{
    X86,
    Amd64
};

Maybe<Architecture> buildArchitecture()
{
    QString arch = QSysInfo::buildCpuArchitecture();
    if(arch == "x86_64")
        return Architecture::Amd64;
    if(arch == "i386")
        return Architecture::X86;
    return nullptr;
}

struct Asset
{
    Platform platform;
    Architecture arch;
    QString url;
};

struct Release
{
    Version version;
    bool is_prerelease;
    bool is_draft;
    QString url;
    QVector<Asset> assets;
};

class ReleaseParser final
{
    Q_DISABLE_COPY(ReleaseParser)

public:
    inline ReleaseParser();
    ~ReleaseParser();
    void parse(const QByteArray & _json);
    inline const QVector<const Release *> releases() const;

private:
    Maybe<Version> parseVersionTag(const QString & _tag_name) const;
    void parseAssets(QJsonArray & _jassets, Release & _release);

private:
    QVector<const Release *> m_releases;
};

} // namespace

ReleaseParser::ReleaseParser()
{
}

ReleaseParser::~ReleaseParser()
{
    for(const Release * release : m_releases)
        delete release;
}

void ReleaseParser::parse(const QByteArray & _json)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(_json);
    QJsonArray jroot = jdoc.array();
    for(QJsonValueRef jrelease_ref : jroot)
    {
        QJsonObject jrelease = jrelease_ref.toObject();
        Maybe<Version> version = parseVersionTag(jrelease["tag_name"].toString());
        if(!version.hasValue())
            continue;
        Release * release = new Release;
        release->version = version.value();
        release->is_draft = jrelease["draft"].toBool();
        release->is_prerelease = jrelease["prerelease"].toBool();
        release->url = jrelease["html_url"].toString();
        QJsonArray jassets = jrelease["assets"].toArray();
        parseAssets(jassets, *release);
        m_releases.append(release);
    }
}

Maybe<Version> ReleaseParser::parseVersionTag(const QString & _tag_name) const
{
    QRegExp regex("(\\d+)\\.(\\d+)");
    if(!regex.exactMatch(_tag_name))
        return nullptr;
    bool parsed = false;
    quint16 major = regex.cap(1).toUShort(&parsed);
    if(!parsed)
        return nullptr;
    quint16 minor = regex.cap(2).toUShort(&parsed);
    if(!parsed)
        return nullptr;
    return Version(major, minor);
}

void ReleaseParser::parseAssets(QJsonArray & _jassets, Release & _release)
{
    QRegExp name_regex(".*_(.*)_\\d+\\.\\d+_([^\\.]*).*");
    for(QJsonValueRef jasset_ref : _jassets)
    {
        Asset asset = { };
        QJsonObject jasset = jasset_ref.toObject();
        QString name = jasset["name"].toString();
        if(!name_regex.exactMatch(name))
            continue;
        asset.url = jasset["browser_download_url"].toString();
        QString platform = name_regex.cap(1);
        QString arch = name_regex.cap(2);
        if(platform.compare("linux", Qt::CaseInsensitive) == 0)
            asset.platform = Platform::Linux;
        else if(platform.compare("windows", Qt::CaseInsensitive) == 0)
            asset.platform = Platform::Windows;
        else
            continue;
        if(arch.compare("amd64", Qt::CaseInsensitive) == 0)
            asset.arch = Architecture::Amd64;
        else if(arch.compare("x86", Qt::CaseInsensitive) == 0)
            asset.arch = Architecture::X86;
        else
            continue;
        _release.assets.append(asset);
    }
}

const QVector<const Release *> ReleaseParser::releases() const
{
    return m_releases;
}

Updater::Updater(QObject * _parent /*= nullptr*/) :
    m_current_version(Version(_OPLPCTOOLS_VERSION_MAJOR, _OPLPCTOOLS_VERSION_MINOR)),
    QObject(_parent)
{
}

bool Updater::isSupported()
{
    Maybe<Platform> platform = buildPlatform();
    return platform.hasValue() && buildArchitecture().hasValue() && (
        platform.value() == Platform::Windows ||
        QFileInfo(QCoreApplication::applicationFilePath()).isWritable()
    );
}

void Updater::checkForUpdate()
{
    if(!isSupported())
        return;
    QNetworkAccessManager * network = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("https://api.github.com/repos/brainstream/OPL-PC-Tools/releases"));
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(config);
    connect(network, &QNetworkAccessManager::finished, [this, network](QNetworkReply * reply) {
        if(reply->error() == QNetworkReply::NoError)
        {
            readUpdates(reply->readAll());
            if(m_update.hasValue())
                emit updateAvailable();
        }
        network->deleteLater();
    });
    network->get(request);
}

void Updater::readUpdates(const QByteArray & _releases_json)
{
    ReleaseParser parser;
    parser.parse(_releases_json);
    Platform platform = buildPlatform().value();
    Architecture arch = buildArchitecture().value();
    m_update.reset();
    for(const Release * release : parser.releases())
    {
        if(release->is_draft || release->is_prerelease || release->assets.empty() ||
           release->version <= (m_update.hasValue() ? m_update.value().version : m_current_version))
        {
            continue;
        }
        for(const Asset & asset : release->assets)
        {
            if(asset.arch == arch && asset.platform == platform)
            {
                Update update;
                update.version = release->version;
                update.html_url = release->url;
                update.download_url = asset.url;
                m_update = update;
                break;
            }
        }
    }
}
