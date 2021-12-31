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

#include <QThread>
#include <QDir>
#include <QMessageBox>
#include <OplPcTools/File.h>
#include <OplPcTools/UI/BusyDialog.h>
#include <OplPcTools/UI/VmcExportThread.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

VmcExportThreadWorker::VmcExportThreadWorker() :
    mp_loop(nullptr),
    m_action(Action::Skip)
{
}

void VmcExportThreadWorker::start(const Vmc & _vmc, const QString & _destination_dir)
{
    try
    {
        m_action = Action::Skip;
        QSharedPointer<VmcFS> fs = VmcFS::load(_vmc.filepath());
        exportDirectory(*fs, VmcPath::root(), _destination_dir);
    }
    catch(const Exception & _exception)
    {
        emit exception(_exception.message());
    }
    catch(...)
    {
        emit exception(tr("An unknown error occurred while exporting VMC"));
    }
    emit finished();
}

void VmcExportThreadWorker::exportDirectory(VmcFS & _fs, const VmcPath & _vmc_dir, const QString & _dest_directory)
{
    if(m_action == Action::Cancel)
        return;
    QDir dir(_dest_directory);
    if(!dir.exists() && !QDir().mkpath(dir.path()))
        throw Exception(QObject::tr("Unable to create directory \"%1\"").arg(dir.path()));
    QList<VmcEntryInfo> entries = _fs.enumerateEntries(_vmc_dir);
    for(const VmcEntryInfo & entry : entries)
    {
        VmcPath next_vmc_entry = _vmc_dir + entry.name;
        if(entry.is_directory)
        {
            QString next_directory = dir.absoluteFilePath(entry.name);
            exportDirectory(_fs, next_vmc_entry, next_directory);
        }
        else
        {
            exportFile(_fs, next_vmc_entry, _dest_directory);
            if(m_action == Action::Cancel)
                break;
        }
    }
}

void VmcExportThreadWorker::exportFile(VmcFS & _fs, const VmcPath & _vmc_file, const QString & _dest_directory)
{
    QSharedPointer<VmcFile> file = _fs.openFile(_vmc_file);
    if(!file)
        throw Exception(QObject::tr("Unable to open VMC file \"%1\"").arg(_vmc_file));
    QFile out(QDir(_dest_directory).absoluteFilePath(file->name()));
    if(!out.exists() ||
       getAction(tr("The file \"%1\" exists. Do you want to overwrite it?").arg(out.fileName())) == Action::Overwrite)
    {
        char * buffer = new char[file->size()];
        int64_t size = file->read(buffer, file->size());
        openFile(out, QIODevice::Truncate | QIODevice::WriteOnly);
        if(size > 0)
            out.write(buffer, size);
        delete [] buffer;
    }
}

VmcExportThreadWorker::Action VmcExportThreadWorker::getAction(const QString & _question)
{
    emit askQuestion(_question);
    mp_loop = new QEventLoop(this);
    mp_loop->exec(QEventLoop::ExcludeUserInputEvents);
    delete mp_loop;
    mp_loop = nullptr;
    return m_action;
}

void VmcExportThreadWorker::setAnswer(bool _answer)
{
    m_action = _answer ? Action::Overwrite : Action::Skip;
    if(mp_loop) mp_loop->quit();
}

void VmcExportThreadWorker::cancel()
{
    m_action = Action::Cancel;
    if(mp_loop) mp_loop->quit();
}

VmcExportThread::VmcExportThread(QWidget * _parent_widget) :
    QObject(_parent_widget),
    mp_parent_widget(_parent_widget),
    m_default_answer(nullptr),
    mp_worker(nullptr)
{
}

void VmcExportThread::start(const Vmc & _vmc, const QString & _destination_dir)
{
    QThread * thread = new QThread(this);
    BusyDialog * busy_dialog = new BusyDialog(mp_parent_widget);
    mp_worker = new VmcExportThreadWorker();
    connect(thread, &QThread::started, [this, &_vmc, &_destination_dir]() {
        mp_worker->start(_vmc, _destination_dir);
    });
    connect(thread, &QThread::finished, this, &VmcExportThread::finished);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, mp_worker, &VmcExportThreadWorker::deleteLater);
    connect(thread, &QThread::finished, busy_dialog, &BusyDialog::deleteLater);
    connect(mp_worker, &VmcExportThreadWorker::finished, thread, &QThread::quit);
    connect(mp_worker, &VmcExportThreadWorker::askQuestion, this, &VmcExportThread::askQuestion);
    connect(mp_worker, &VmcExportThreadWorker::exception, this, &VmcExportThread::exception);
    m_default_answer.reset();
    mp_worker->moveToThread(thread);
    thread->start();
    busy_dialog->exec();
}

void VmcExportThread::askQuestion(const QString & _question)
{
    if(m_default_answer.has_value())
    {
        mp_worker->setAnswer(m_default_answer.value());
    }
    else
    {
        int btn = QMessageBox::question(
            mp_parent_widget, tr("Overwrite confirmation"), _question,
            QMessageBox::Yes | QMessageBox::No | QMessageBox::YesAll | QMessageBox::NoAll | QMessageBox::Cancel);
        switch(btn)
        {
        case QMessageBox::Cancel:
            mp_worker->cancel();
            break;
        case QMessageBox::Yes:
            mp_worker->setAnswer(true);
            break;
        case QMessageBox::YesAll:
            m_default_answer = true;
            mp_worker->setAnswer(true);
            break;
        case QMessageBox::NoAll:
            m_default_answer = false;
            mp_worker->setAnswer(false);
            break;
        default:
            mp_worker->setAnswer(false);
            break;
        }
    }
}
