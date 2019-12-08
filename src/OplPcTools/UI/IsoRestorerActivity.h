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

#ifndef __OPLPCTOOLS_ISORESTORERACTIVITY__
#define __OPLPCTOOLS_ISORESTORERACTIVITY__

#include <QThread>
#include <QWidget>
#include <QSharedPointer>
#include <OplPcTools/Game.h>
#include <OplPcTools/UI/Intent.h>
#include "ui_IsoRestorerActivity.h"

namespace OplPcTools {
namespace UI {

class IsoRestorerActivity : public Activity, private Ui::IsoRestorerActivity
{
    Q_OBJECT

public:
    explicit IsoRestorerActivity(const QString & _game_id, QWidget * _parent = nullptr);
    bool onAttach() override;

    static QSharedPointer<Intent> createIntent(const QString & _game_id);

private:
    void restore(const Game & _game, const QString & _destination);

private slots:
    void onProgress(quint64 _total_bytes, quint64 _processed_bytes);
    void onRollbackStarted();
    void onException(QString _message);
    void onThreadFinished();
    void onCancel();

private:
    static const quint32 s_progress_max = 1000;
    const QString m_game_id;
    QThread * mp_working_thread;
    QString m_finish_status;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_ISORESTORERACTIVITY__
