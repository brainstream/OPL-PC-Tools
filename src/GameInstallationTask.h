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

#ifndef __QPCOPL_GAMEINSTALLATIONTASK__
#define __QPCOPL_GAMEINSTALLATIONTASK__

#include "Iso9660GameInstallerSource.h"
#include "OpticalDiscGameInstallerSource.h"

enum class GameInstallationStatus
{
    queued,
    installation,
    registration,
    done,
    error,
    rollingBack
};

class GameInstallationTask
{
public:
    GameInstallationTask();
    GameInstallationTask(const GameInstallationTask &) = default;
    virtual ~GameInstallationTask() { }
    GameInstallationTask & operator = (const GameInstallationTask &) = default;
    virtual GameInstallerSource * createSource() const = 0;
    virtual bool canChangeMediaType() const = 0;
    inline const QString & gameName() const;
    inline void setGameName(const QString & _name);
    inline GameInstallationStatus status() const;
    inline void setStatus(GameInstallationStatus _status);
    inline const QString & errorMessage() const;
    inline void setErrorMessage(const QString & _message);
    inline void setErrorStatus(const QString & _message);
    inline MediaType mediaType() const;
    inline bool setMediaType(MediaType & _media_type);

protected:
    inline void completeSource(GameInstallerSource & _source) const;

private:
    QString m_game_name;
    GameInstallationStatus m_status;
    QString m_error_message;
    MediaType m_media_type;
};

GameInstallationTask::GameInstallationTask() :
    m_media_type(MediaType::unknown)
{
}

const QString & GameInstallationTask::gameName() const
{
    return m_game_name;
}

void GameInstallationTask::setGameName(const QString & _name)
{
    m_game_name = _name;
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
    m_status = GameInstallationStatus::error;
    m_error_message = _message;
}

MediaType GameInstallationTask::mediaType() const
{
    return m_media_type;
}

bool GameInstallationTask::setMediaType(MediaType & _media_type)
{
    if(canChangeMediaType())
    {
        m_media_type = _media_type;
        return true;
    }
    return false;
}

void GameInstallationTask::completeSource(GameInstallerSource & _source) const
{
    _source.setGameName(m_game_name);
}

class Iso9660GameInstallationTask : public GameInstallationTask
{
public:
    explicit inline Iso9660GameInstallationTask(const QString & _iso_path);
    Iso9660GameInstallationTask(const Iso9660GameInstallationTask &) = default;
    Iso9660GameInstallationTask & operator = (const Iso9660GameInstallationTask &) = default;
    inline GameInstallerSource * createSource() const override;
    inline bool canChangeMediaType() const override;
    inline const QString & isoPath() const;

private:
    QString m_iso_path;
};

Iso9660GameInstallationTask::Iso9660GameInstallationTask(const QString & _iso_path) :
    m_iso_path(_iso_path)
{
}

GameInstallerSource * Iso9660GameInstallationTask::createSource() const
{
    Iso9660GameInstallerSource * source = new Iso9660GameInstallerSource(m_iso_path);
    source->setType(mediaType());
    completeSource(*source);
    return source;
}

bool Iso9660GameInstallationTask::canChangeMediaType() const
{
    return true;
}

const QString & Iso9660GameInstallationTask::isoPath() const
{
    return m_iso_path;
}

class OpticalDiscGameInstallationTask : public GameInstallationTask
{
public:
    explicit inline OpticalDiscGameInstallationTask(const QString & _device);
    OpticalDiscGameInstallationTask(const OpticalDiscGameInstallationTask &) = default;
    OpticalDiscGameInstallationTask & operator = (const OpticalDiscGameInstallationTask &) = default;
    inline GameInstallerSource * createSource() const override;
    inline bool canChangeMediaType() const override;
    inline const QString & device() const;

private:
    QString m_device;
};

OpticalDiscGameInstallationTask::OpticalDiscGameInstallationTask(const QString & _device) :
    m_device(_device)
{
}

GameInstallerSource * OpticalDiscGameInstallationTask::createSource() const
{
    OpticalDiscGameInstallerSource * source = new OpticalDiscGameInstallerSource(m_device.toUtf8());
    completeSource(*source);
    return source;
}

bool OpticalDiscGameInstallationTask::canChangeMediaType() const
{
    return false;
}

const QString & OpticalDiscGameInstallationTask::device() const
{
    return m_device;
}

#endif // __QPCOPL_GAMEINSTALLATIONTASK__
