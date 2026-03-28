/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
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
 * You should have received a copy of the GNU General Public License along with OPL PC Tools   *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <OplPcTools/UI/VmcExporter.h>
#include <OplPcTools/UI/ProgressDialog.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/File.h>
#include <QMessageBox>
#include <QThread>
#include <QDir>

using namespace OplPcTools;
using namespace OplPcTools::UI;

VmcExporter::VmcExporter(
    const Vmc & _vmc,
    const StringConverter & _string_converter,
    const QList<MemoryCard::Path> & _sources,
    const QString & _destination_dir
) :
    m_action(Action::Skip),
    mr_vmc(_vmc),
    mr_string_converter(_string_converter),
    mr_sources(_sources),
    m_destination_dir(_destination_dir)
{
}

void VmcExporter::run()
{
    m_action = Action::Skip;
    std::unique_ptr<MemoryCard::FileSystem> fs(new MemoryCard::FileSystem(mr_vmc.filepath(), nullptr));
    fs->load();
    foreach(const MemoryCard::Path & source, mr_sources)
    {
        if(std::optional<MemoryCard::EntryInfo> entry = fs->entry(source))
        {
            if(entry->isDirectory())
                exportDirectory(*fs, mr_string_converter, MemoryCard::Path::root(), m_destination_dir);
            else
                exportFile(*fs, mr_string_converter, source, m_destination_dir);
        }
        else
        {
            throw MemoryCard::MemoryCardFileException(QObject::tr("Source path not found"), source.path());
        }
    }
}

void VmcExporter::exportDirectory(
    MemoryCard::FileSystem & _fs,
    const StringConverter & _string_converter,
    const MemoryCard::Path & _vmc_dir,
    const QString & _dest_directory)
{
    if(m_action == Action::Cancel)
        return;
    QDir dir(_dest_directory);
    if(!dir.exists() && !QDir().mkpath(dir.path()))
        throw Exception(QObject::tr("Unable to create directory \"%1\"").arg(dir.path()));
    QList<MemoryCard::EntryInfo> entries = _fs.enumerateEntries(_vmc_dir);
    foreach(const MemoryCard::EntryInfo & entry, entries)
    {
        MemoryCard::Path next_vmc_entry = _vmc_dir + entry.name();
        if(entry.isDirectory())
        {
            QString next_directory = dir.absoluteFilePath(entry.name());
            exportDirectory(_fs, _string_converter, next_vmc_entry, next_directory);
        }
        else
        {
            exportFile(_fs, _string_converter, next_vmc_entry, _dest_directory);
            if(m_action == Action::Cancel)
                break;
        }
    }
}

void VmcExporter::exportFile(
    MemoryCard::FileSystem & _fs,
    const StringConverter & _string_converter,
    const MemoryCard::Path & _vmc_file,
    const QString & _dest_directory)
{
    QSharedPointer<MemoryCard::File> file = _fs.openFile(_vmc_file);
    if(!file)
        throw MemoryCard::MemoryCardFileException(QObject::tr("Unable to open VMC file"), _vmc_file.path());
    QFile out(QDir(_dest_directory).absoluteFilePath(_string_converter.decode(file->name())));
    if(
        !out.exists() ||
        Action::Overwrite == getAction(
            QObject::tr("The file \"%1\" exists. Do you want to overwrite it?").arg(out.fileName())))
    {
        char * buffer = new char[file->size()];
        int64_t size = file->read(buffer, file->size());
        openFile(out, QIODevice::Truncate | QIODevice::WriteOnly);
        if(size > 0)
            out.write(buffer, size);
        delete [] buffer;
    }
}

VmcExporter::Action VmcExporter::getAction(const QString & _question)
{
    if(m_default_answer.has_value())
    {
        return m_default_answer.value() ? Action::Overwrite : Action::Skip;
    }
    else
    {
        QMessageBox::StandardButton btn = Application::askQuestion(
            QObject::tr("Overwrite confirmation"),
            _question,
            QMessageBox::Yes | QMessageBox::No | QMessageBox::YesAll | QMessageBox::NoAll | QMessageBox::Cancel);
        switch(btn)
        {
        case QMessageBox::Cancel:
            return Action::Cancel;
        case QMessageBox::Yes:
            return Action::Overwrite;
        case QMessageBox::YesAll:
            m_default_answer = true;
            return Action::Overwrite;
        case QMessageBox::NoAll:
            m_default_answer = false;
            return Action::Skip;
        default:
            return Action::Skip;
        }
    }
}
