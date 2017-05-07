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
#include "GameRepository.h"
#include "GameInstallerSource.h"

class GameInstaller : public QObject
{
    Q_OBJECT

public:
    GameInstaller(GameInstallerSource & _source, GameRepository & _repository, QObject * _parent = nullptr);
    ~GameInstaller() override;
    inline const GameInstallerSource & source() const;
    bool install();
    inline const Game * installedGame() const;

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
    GameRepository & mr_repository;
    QStringList m_written_parts;
    Game * mp_installed_game;
};

const GameInstallerSource & GameInstaller::source() const
{
    return *mp_sourrce;
}

const Game * GameInstaller::installedGame() const
{
    return mp_installed_game;
}

#endif // __QPCOPL_GAMEINSTALLER__
