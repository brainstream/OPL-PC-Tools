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

#ifndef __QPCOPL_OPTICALDISCGAMEINSTALLERSOURCE__
#define __QPCOPL_OPTICALDISCGAMEINSTALLERSOURCE__

#include "GameInstallerSource.h"

class OpticalDiscGameInstallerSource  : public GameInstallerSource
{
    Q_DISABLE_COPY(OpticalDiscGameInstallerSource)

private:
    struct Data;

public:
    explicit OpticalDiscGameInstallerSource(const char * _device);
    ~OpticalDiscGameInstallerSource() override;
    QString gameId() const override;
    size_t read(QByteArray & _buffer) override;
    quint64 size() const override;
    MediaType type() const override;

protected:
    QByteArray read(lsn_t _lsn, quint32 _blocks) const override;

private:
    void init() const;
    void initSize() const;
    void initGameId() const;
    void initLabel() const;
    void initMediaType() const;

private:
    Data * mp_data;
};

#endif // __QPCOPL_OPTICALDISCGAMEINSTALLERSOURCE__
