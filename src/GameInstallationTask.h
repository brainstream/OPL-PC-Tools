/***********************************************************************************************
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

#ifndef __OPLPCTOOLS_GAMEINSTALLATIONTASK__
#define __OPLPCTOOLS_GAMEINSTALLATIONTASK__

#include <QSharedPointer>
#include "Device.h"

enum class GameInstallationStatus
{
    Queued,
    Installation,
    Registration,
    Done,
    Error,
    RollingBack
};

class GameInstallationTask
{
public:
    explicit GameInstallationTask(QSharedPointer<Device> & _device);
    GameInstallationTask(const GameInstallationTask &) = default;
    virtual ~GameInstallationTask() { }
    GameInstallationTask & operator = (const GameInstallationTask &) = default;
    inline Device & device();
    inline const Device & device() const;
    inline GameInstallationStatus status() const;
    inline void setStatus(GameInstallationStatus _status);
    inline const QString & errorMessage() const;
    inline void setErrorMessage(const QString & _message);
    inline void setErrorStatus(const QString & _message);

private:
    QSharedPointer<Device> m_device_ptr;
    GameInstallationStatus m_status;
    QString m_error_message;
};

GameInstallationTask::GameInstallationTask(QSharedPointer<Device> & _device) :
    m_device_ptr(_device),
    m_status(GameInstallationStatus::Queued)
{
}

Device & GameInstallationTask::device()
{
    return *m_device_ptr;
}

const Device & GameInstallationTask::device() const
{
    return *m_device_ptr;
}

GameInstallationStatus GameInstallationTask::status() const
{
    return m_status;
}

void GameInstallationTask::setStatus(GameInstallationStatus _status)
{
    m_status = _status;
}

const QString & GameInstallationTask::errorMessage() const
{
    return m_error_message;
}

void GameInstallationTask::setErrorMessage(const QString & _message)
{
    m_error_message = _message;
}

void GameInstallationTask::setErrorStatus(const QString & _message)
{
    m_status = GameInstallationStatus::Error;
    m_error_message = _message;
}

#endif // __OPLPCTOOLS_GAMEINSTALLATIONTASK__
