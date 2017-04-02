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

#ifndef __QPCOPL_ISO9660GAMGEINSTALLERSOURCE__
#define __QPCOPL_ISO9660GAMGEINSTALLERSOURCE__

#include <QFile>
#include "GameInstallerSource.h"

class Iso9660GameInstallerSource : public GameInstallerSource
{
    Q_DISABLE_COPY(Iso9660GameInstallerSource)

public:
    explicit Iso9660GameInstallerSource(const QString & _iso_path);
    QString gameId() const override;
    QByteArray read(quint64 _length) override;
    quint64 size() const override;
    void setType(MediaType _type);
    MediaType type() const override;

private:
    QFile m_file;
    MediaType m_type;
};

#endif // __QPCOPL_ISO9660GAMGEINSTALLERSOURCE__
