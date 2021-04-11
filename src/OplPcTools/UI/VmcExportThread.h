/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_VMCEXPORTTHREAD__
#define __OPLPCTOOLS_VMCEXPORTTHREAD__

#include <optional>
#include <QWidget>
#include <QEventLoop>
#include <OplPcTools/Vmc.h>
#include <OplPcTools/VmcFS.h>

namespace OplPcTools {
namespace UI {


class VmcExportThreadWorker : public QObject
{
    Q_OBJECT

    enum class Action { Overwrite, Skip, Cancel };

public:
    VmcExportThreadWorker();
    void start(const Vmc & _vmc, const QString & _destination_dir);
    void setAnswer(bool _answer);
    void cancel();

private:
    void exportDirectory(VmcFS & _fs, const QString & _vmc_dir, const QString & _dest_directory);
    void exportFile(VmcFS & _fs, const QString & _vmc_file, const QString & _dest_directory);
    Action getAction(const QString & _question);

signals:
    void askQuestion(const QString & _question);
    void finished();
    void exception(const QString & _message);

private:
    QEventLoop * mp_loop;
    Action m_action;
};


class VmcExportThread : public QObject
{
    Q_OBJECT

public:
    explicit VmcExportThread(QWidget * _parent_widget);
    void start(const Vmc & _vmc, const QString & _destination_dir);

signals:
    void finished();
    void exception(const QString & _message);

private:
    void askQuestion(const QString & _question);

private:
    QWidget * mp_parent_widget;
    std::optional<bool> m_default_answer;
    VmcExportThreadWorker * mp_worker;
};


} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCEXPORTTHREAD__
