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

#include <OplPcTools/UI/VmcPropertiesDialog.h>
#include <OplPcTools/UI/DisplayUtils.h>
#include <OplPcTools/MemoryCard/FileSystem.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/Settings.h>

using namespace OplPcTools::UI;

namespace {

inline QString numberToHexString(uint32_t _number)
{
    return QString("0x%1").arg(_number, 0, 16);
}

} // namespace

VmcPropertiesDialog::VmcPropertiesDialog(const Vmc & _vmc, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent)
{
    setupUi(this);
    mp_label_error_message->hide();
    mp_tabs->hide();
    try
    {
        QSharedPointer<MemoryCard::FileSystem> fs(new MemoryCard::FileSystem(_vmc.filepath()));
        fs->load();
        const MemoryCard::FSInfo * info = fs->info();
        mp_value_magic->setText(info->magic);
        mp_value_version->setText(info->version);
        mp_value_pagesize->setText(QString::number(info->pagesize));
        mp_value_pages_per_cluster->setText(QString::number(info->pages_per_cluster));
        mp_value_pages_per_block->setText(QString::number(info->pages_per_block));
        mp_value_clusters_per_card->setText(QString::number(info->clusters_per_card));
        mp_value_alloc_offset->setText(QString::number(info->alloc_offset));
        mp_value_alloc_end->setText(QString::number(info->alloc_end));
        mp_value_root_dir_cluster->setText(QString::number(info->rootdir_cluster));
        mp_value_backup_block1->setText(QString::number(info->backup_block1));
        mp_value_backup_block2->setText(QString::number(info->backup_block2));
        mp_value_card_type->setText(QString::number(info->cardtype));
        mp_value_card_flags->setText(numberToHexString(info->cardflags));
        mp_value_cluster_size->setText(QString::number(info->cluster_size));
        mp_value_fat_entries_per_cluster->setText(QString::number(info->fat_entries_per_cluster));
        mp_value_clusters_per_block->setText(QString::number(info->clusters_per_block));
        mp_value_card_format->setText(QString::number(info->cardform));
        mp_value_max_allocatable_clusters->setText(QString::number(info->max_allocatable_clusters));
        mp_value_file->setText(_vmc.filepath());
        mp_value_title->setText(_vmc.title());
        mp_value_size->setText(QString("%1 %2").arg(_vmc.size()).arg(tr("MiB")));
        mp_value_used->setText(makeBytesDisplayString(fs->totalUsedBytes()));
        mp_value_free->setText(makeBytesDisplayString(fs->totalFreeBytes()));
        {
            QString charset = Library::instance().config().vmcFsCharset(_vmc);
            if(charset.isEmpty())
                charset = Settings::instance().defaultVmcFsCharset();
            mp_value_fs_charset->setText(charset);
        }
        QStringList values;
        for(size_t i = 0; i < sizeof(info->bad_block_list) / sizeof(int32_t); ++i)
            values << QString::number(info->bad_block_list[i]);
        mp_value_bad_blocks->setText(values.join(", "));
        values.clear();
        for(size_t i = 0; i < sizeof(info->ifc_ptr_list) / sizeof(uint32_t); ++i)
        {
            uint32_t value = info->ifc_ptr_list[i];
            values << (value == static_cast<uint32_t>(-1) ? QString("-1") : QString::number(value));
        }
        mp_value_ifc_list->setText(values.join(", "));
        mp_tabs->show();
    }
    catch(const Exception & exception)
    {
        mp_label_error_message->setText(exception.message());
        mp_label_error_message->show();
    }
    catch(...)
    {
        mp_label_error_message->setText(tr("Unable to read VMC"));
        mp_label_error_message->show();
    }
    connect(mp_btnbox, &QDialogButtonBox::rejected, this, &VmcPropertiesDialog::reject);
    connect(mp_btnbox, &QDialogButtonBox::accepted, this, &VmcPropertiesDialog::accept);
}
