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

#include <QPushButton>
#include <QCloseEvent>
#include <OplPcTools/Exception.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/FilenameValidator.h>
#include <OplPcTools/UI/LambdaThread.h>
#include <OplPcTools/UI/VmcCreateDialog.h>

using namespace OplPcTools::UI;

VmcCreateDialog::VmcCreateDialog(QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    m_is_in_progress(false)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    setWindowFlag(Qt::WindowCloseButtonHint, false);
#endif
    setupUi(this);
    mp_edit_title->setValidator(new FilenameValidator(this));
    mp_combobox_size->addItem(tr("8 MiB"), 8);
    mp_combobox_size->addItem(tr("16 MiB"), 16);
    mp_combobox_size->addItem(tr("32 MiB"), 32);
    mp_combobox_size->addItem(tr("64 MiB"), 64);
    mp_combobox_size->addItem(tr("128 MiB"), 128);
    mp_combobox_size->addItem(tr("256 MiB"), 256);
    mp_combobox_size->addItem(tr("512 MiB"), 512);
    connect(mp_button_box, &QDialogButtonBox::accepted, this, &VmcCreateDialog::create);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &VmcCreateDialog::reject);
    connect(mp_edit_title, &QLineEdit::textChanged, this, &VmcCreateDialog::setSaveButtonState);
    setProgressVisibility();
    setSaveButtonState();
}

void VmcCreateDialog::setTitle(const QString & _title)
{
    mp_edit_title->setText(_title);
    mp_edit_title->selectAll();
}

void VmcCreateDialog::create()
{
    m_is_in_progress = true;
    setProgressVisibility();
    QString title = mp_edit_title->text();
    uint32_t size = mp_combobox_size->currentData().toUInt();
    LambdaThread * thread = new LambdaThread([this, title, size]() {
        mp_created_vmc = Library::instance().vmcs().createVmc(title, size);
    }, this);
    connect(thread, &LambdaThread::exception, [this](const QString & message) {
        m_is_in_progress = false;
        setProgressVisibility();
        mp_label_error_message->setText(message);
    });
    connect(thread, &LambdaThread::finished, this, &VmcCreateDialog::accept);
    connect(thread, &LambdaThread::finished, [this, thread]() {
        thread->deleteLater();
        m_is_in_progress = false;
    });
    thread->start();
}

void VmcCreateDialog::setProgressVisibility()
{
    mp_widget_main->setHidden(m_is_in_progress);
    mp_widget_progress->setVisible(m_is_in_progress);
    adjustSize();
}

void VmcCreateDialog::setSaveButtonState()
{
    QString filename = mp_edit_title->text().trimmed();
    mp_button_box->button(QDialogButtonBox::Save)->setDisabled(filename.isEmpty());
}

void VmcCreateDialog::closeEvent(QCloseEvent * _event)
{
    if(m_is_in_progress)
        _event->ignore();
    else
        QDialog::closeEvent(_event);
}
