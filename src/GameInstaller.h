/***********************************************************************************************
 *                                                                                             *
 * This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.        *
 *                                                                                             *
 * qpcopl is free software: you can redistribute it and/or modify it under the terms of        *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;        *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#ifndef __QPCOPL_GAMEINSTALLER__
#define __QPCOPL_GAMEINSTALLER__

#include <QObject>
#include <QStringList>
#include "UlConfig.h"
#include "GameInstallerSource.h"

class GameInstaller : public QObject
{
    Q_OBJECT

public:
    GameInstaller(GameInstallerSource & _source, const QString _dest_dir_path, QObject * _parent = nullptr);
    ~GameInstaller() override;
    inline void setGameName(const QString & _name);
    inline const QString & gameName() const;
    inline const GameInstallerSource & source() const;
    inline void setDestinationDirectoryPath(const QString & _path);
    inline const QString & destinationDirectoryPath() const;
    bool install();
    inline const Ul::ConfigRecord * installedGameInfo() const;

signals:
    void progress(quint64 _total_bytes, quint64 _done_bytes);
    void rollbackStarted();
    void rollbackFinished();

private:
    quint32 crc32(const QString & _string);
    void rollback();

private:
    GameInstallerSource * mp_sourrce;
    QString m_dest_dir_path;
    QString m_game_name;
    QStringList m_written_parts;
    Ul::ConfigRecord * mp_installed_game_info;
};

void GameInstaller::setGameName(const QString & _name)
{
    m_game_name = _name;
}

const QString & GameInstaller::gameName() const
{
    return m_game_name;
}

const GameInstallerSource & GameInstaller::source() const
{
    return *mp_sourrce;
}

void GameInstaller::setDestinationDirectoryPath(const QString & _path)
{
    m_dest_dir_path = _path;
}

const QString & GameInstaller::destinationDirectoryPath() const
{
    return m_dest_dir_path;
}

const Ul::ConfigRecord * GameInstaller::installedGameInfo() const
{
    return mp_installed_game_info;
}

#endif // __QPCOPL_GAMEINSTALLER__
