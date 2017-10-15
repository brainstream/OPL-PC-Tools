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

#ifndef __OPLPCTOOLS_ISORECOVERDIALOG__
#define __OPLPCTOOLS_ISORECOVERDIALOG__

#include <OplPcTools/Core/IsoRecoverer.h>
#include <OplPcTools/UI/LambdaThread.h>
#include "ui_IsoRecoverDialog.h"

class IsoRecoverDialog : public QDialog, private Ui::IsoRecoverDialog
{
    Q_OBJECT

public:
    IsoRecoverDialog(const Game & _game, const QString & _game_dirpath, const QString & _iso_filepath, QWidget * _parent);
    ~IsoRecoverDialog() override;

public slots:
    void reject() override;

protected:
    void showEvent(QShowEvent * _event) override;

private slots:
    void recoveryProgress(quint64 _total_bytes, quint64 _processed_bytes);
    void rollbackStarted();
    void threadFinished();
    void recoveryError(QString _message);

private:
    QWidget * mp_parent;
    DialogCode m_return_code;
    static const quint32 s_progress_max = 1000;
    LambdaThread * mp_work_thread;
    IsoRecoverer * mp_recoverer;
};

#endif // __OPLPCTOOLS_ISORECOVERDIALOG__
