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

#ifndef __OPLPCTOOLS_UPDATER__
#define __OPLPCTOOLS_UPDATER__

#include <QObject>
#include <OplPcTools/Maybe.h>

#ifdef major
// https://bugzilla.redhat.com/show_bug.cgi?id=130601
#undef major
#undef minor
#endif

namespace OplPcTools {

class Version
{
public:
    Version() :
        Version(0, 0)
    {
    }

    Version(quint16 _major, quint16 _minor) :
        m_major(_major),
        m_minor(_minor)
    {
    }

    Version(const Version & _ver) = default;

    Version & operator = (const Version & _ver) = default;

    bool operator < (const Version & _ver) const
    {
        return m_major < _ver.m_major || (m_major == _ver.m_major && m_minor < _ver.m_minor);
    }

    bool operator <= (const Version & _ver) const
    {
        return m_major < _ver.m_major || (m_major == _ver.m_major && m_minor <= _ver.m_minor);
    }

    bool operator > (const Version & _ver) const
    {
        return m_major > _ver.m_major || (m_major == _ver.m_major && m_minor > _ver.m_minor);
    }

    bool operator >= (const Version & _ver) const
    {
        return m_major > _ver.m_major || (m_major == _ver.m_major && m_minor >= _ver.m_minor);
    }

    bool operator == (const Version & _ver) const
    {
        return m_major == _ver.m_major && m_minor == _ver.m_minor;
    }

    bool operator != (const Version & _ver) const
    {
        return !this->operator == (_ver);
    }

    void setVersion(quint16 _major, quint16 _minor)
    {
        m_major = _major;
        m_minor = _minor;
    }

    void setMajor(quint16 _major)
    {
        m_major = _major;
    }

    quint16 major() const
    {
        return m_major;
    }

    void setMinor(quint16 _minor)
    {
        m_minor = _minor;
    }

    quint16 minor() const
    {
        return m_minor;
    }

private:
    quint16 m_major;
    quint16 m_minor;
};

struct Update
{
    Version version;
    QString download_url;
    QString html_url;
};

class Updater : public QObject
{
    Q_OBJECT

public:
    explicit Updater(QObject * _parent = nullptr);
    void checkForUpdate();

    const Update * latestUpdate() const
    {
        return m_update.hasValue() ? &m_update.value() : nullptr;
    }

    static bool isSupported();

signals:
    void updateAvailable();

private:
    void readUpdates(const QByteArray & _releases_json);

private:
    const Version m_current_version;
    Maybe<Update> m_update;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_UPDATER__
