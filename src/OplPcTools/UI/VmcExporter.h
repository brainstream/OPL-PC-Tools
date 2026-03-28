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

#ifndef __OPLPCTOOLS_VMCEXPORTTHREAD__
#define __OPLPCTOOLS_VMCEXPORTTHREAD__

#include <OplPcTools/StringConverter.h>
#include <OplPcTools/Vmc.h>
#include <OplPcTools/MemoryCard/FileSystem.h>
#include <optional>

namespace OplPcTools {
namespace UI {

class VmcExporter
{
    Q_DISABLE_COPY_MOVE(VmcExporter)

private:
    enum class Action { Overwrite, Skip, Cancel };

public:
    VmcExporter(
        const Vmc & _vmc,
        const StringConverter & _string_converter,
        const QList<MemoryCard::Path> & _sources,
        const QString & _destination_dir);
    void run();

private:
    void setAnswer(bool _answer);
    void exportDirectory(
        MemoryCard::FileSystem & _fs,
        const StringConverter & _string_converter,
        const MemoryCard::Path & _vmc_dir,
        const QString & _dest_directory);
    void exportFile(
        MemoryCard::FileSystem & _fs,
        const StringConverter & _string_converter,
        const MemoryCard::Path & _vmc_file,
        const QString & _dest_directory);
    void exportEntries(
        MemoryCard::FileSystem & _fs,
        const StringConverter & _string_converter,
        const MemoryCard::Path & _vmc_dir,
        const QString & _dest_directory);
    Action getAction(const QString & _question);

private:
    std::optional<bool> m_default_answer;
    Action m_action;
    const Vmc & mr_vmc;
    const StringConverter & mr_string_converter;
    const QList<MemoryCard::Path> & mr_sources;
    const QString m_destination_dir;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCEXPORTTHREAD__
