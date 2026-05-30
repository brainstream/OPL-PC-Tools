/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
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
 * You should have received a copy of the GNU General Public License along with OPL PC Tools   *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <OplPcTools/DirectoryGameStorage.h>
#include <OplPcTools/Device/Iso9660DeviceSource.h>
#include <OplPcTools/Device/ZsoDeviceSource.h>
#include <OplPcTools/Device/DeviceReader.h>
#include <OplPcTools/File.h>
#include <OplPcTools/StandardPaths.h>
#include <QVector>

using namespace OplPcTools;

DirectoryGameStorage::DirectoryGameStorage(QObject * _parent /*= nullptr*/) :
    GameStorage(_parent)
{
}

GameInstallationType DirectoryGameStorage::installationType() const
{
    return GameInstallationType::Directory;
}

bool DirectoryGameStorage::performLoading(const QDir & _directory)
{
    m_base_directory = _directory.absolutePath();
    loadDirectory(MediaType::CD);
    loadDirectory(MediaType::DVD);
    return true;
}

void DirectoryGameStorage::loadDirectory(MediaType _media_type)
{
    QDir base_directory(m_base_directory);
    if(!base_directory.cd(_media_type == MediaType::CD ? StandardDirectories::cd : StandardDirectories::dvd))
        return;
    foreach(const QString & iso, base_directory.entryList({ g_filename_pattern_iso, g_filename_pattern_zso }))
    {
        DeviceSource * device_source = nullptr;
        bool is_compressed = QFileInfo(iso).suffix().compare("zso", Qt::CaseInsensitive) == 0;
        if(is_compressed)
            device_source = new ZsoDeviceSource(base_directory.absoluteFilePath(iso));
        else
            device_source = new Iso9660DeviceSource(base_directory.absoluteFilePath(iso));
        DeviceReader image{QSharedPointer<DeviceSource>(device_source)};
        if(!image.init())
            continue;
        Game * game = createGame(image.gameId());
        game->setMediaType(_media_type);
        game->setPartCount(1);
        game->setIsCompressed(is_compressed);
        QString title = QFileInfo(image.filepath()).fileName();
        title = title.left(title.lastIndexOf('.'));
        if(title.startsWith(image.gameId()))
            game->setTitle(title.right(title.size() - image.gameId().size() - 1));
        else
            game->setTitle(title);
    }
}

bool DirectoryGameStorage::performRenaming(const Game & _game, const QString & _title)
{
    if(_game.title() == _title)
        return true;
    std::optional<FindIsoResult> find_result = findIsoFile(m_base_directory, _game);
    if(!find_result)
        return false;
    const QString ext = getFilenameExtension(_game);
    const QString new_filename = QFileInfo(find_result->path).dir().absoluteFilePath(
        find_result->is_name_included_id ? makeIsoFilename(_title, _game.id(), ext) : makeIsoFilename(_title, ext));
    return QFile::rename(find_result->path, new_filename);
}

std::optional<DirectoryGameStorage::FindIsoResult> DirectoryGameStorage::findIsoFile(
    const QString & _storage_directory,
    const Game & _game)
{
    QDir directory(_storage_directory);
    if(!directory.cd(_game.mediaType() == MediaType::CD ? StandardDirectories::cd : StandardDirectories::dvd))
        return std::nullopt;
    QString ext = getFilenameExtension(_game);
    QString filename = directory.absoluteFilePath(makeIsoFilename(_game.title(), _game.id(), ext));
    bool is_name_included_id = true;
    if(!QFile::exists(filename))
    {
        is_name_included_id = false;
        filename = directory.absoluteFilePath(makeIsoFilename(_game.title(), ext));
        if(!QFile::exists(filename))
            return std::nullopt;
    }
    return FindIsoResult
    {
        .path = filename,
        .is_name_included_id = is_name_included_id
    };
}

QString DirectoryGameStorage::getFilenameExtension(const Game & _game)
{
    return _game.isCompressed() ? g_file_ext_zso : g_file_ext_iso;
}

bool DirectoryGameStorage::performRegistration(const Game & _game)
{
    Q_UNUSED(_game)
    return true;
}

QString DirectoryGameStorage::makeIsoFilename(const QString & _title, const QString & _id, const QString _ext)
{
    return QString("%1.%2%3").arg(_id, _title, _ext);
}

QString DirectoryGameStorage::makeIsoFilename(const QString & _title, const QString _ext)
{
    return _title + _ext;
}

QString DirectoryGameStorage::makeGameIsoFilename(const QString & _title, const QString & _id, const QString _ext)
{
    return QString("%1.%2%3").arg(_id, _title, _ext);
}

bool DirectoryGameStorage::performDeletion(const Game & _game)
{
    std::optional<FindIsoResult> find_result = findIsoFile(m_base_directory, _game);
    return find_result && QFile::remove(find_result->path);
}
