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
    GameInstaller(GameInstallerSource & _source, UlConfig & _config, QObject * _parent = nullptr);
    ~GameInstaller() override;
    inline const GameInstallerSource & source() const;
    bool install();
    inline const UlConfigRecord * installedGameInfo() const;

signals:
    void progress(quint64 _total_bytes, quint64 _done_bytes);
    void registrationStarted();
    void registrationFinished();
    void rollbackStarted();
    void rollbackFinished();

private:
    void rollback();
    void registerGame();

private:
    GameInstallerSource * mp_sourrce;
    UlConfig & mr_config;
    QStringList m_written_parts;
    UlConfigRecord * mp_installed_game_info;
};

const GameInstallerSource & GameInstaller::source() const
{
    return *mp_sourrce;
}

const UlConfigRecord * GameInstaller::installedGameInfo() const
{
    return mp_installed_game_info;
}

#endif // __QPCOPL_GAMEINSTALLER__
