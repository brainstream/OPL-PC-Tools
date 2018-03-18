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

#include <cmath>
#include <QFileDialog>
#include <QSettings>
#include <OplPcTools/Core/IsoRestorer.h>
#include <OplPcTools/Core/Game.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/LambdaThread.h>
#include <OplPcTools/UI/IsoRestorerWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {

static const char * iso_recover_dir = "ISORecoverDirectory";

} // namespace SettingsKey

class IsoRestorerWidgetIntent : public Intent
{
public:
    explicit IsoRestorerWidgetIntent(const QString & _game_id) :
        m_game_id(_game_id)
    {
    }

    Activity * createActivity(QWidget * _parent) override
    {
        IsoRestorerWidget * widget = new IsoRestorerWidget(m_game_id, _parent);
        return widget;
    }

private:
    const QString m_game_id;
};

} // namespace

IsoRestorerWidget::IsoRestorerWidget(const QString & _game_id, QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    m_game_id(_game_id),
    mp_working_thread(nullptr)
{
    setupUi(this);
    mp_btn_back->setDisabled(true);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &IsoRestorerWidget::onCancel);
    connect(mp_btn_back, &QPushButton::clicked, this, &IsoRestorerWidget::deleteLater);
}

QSharedPointer<Intent> IsoRestorerWidget::createIntent(const QString & _game_id)
{
    return QSharedPointer<Intent>(new IsoRestorerWidgetIntent(_game_id));
}

bool IsoRestorerWidget::onAttach()
{
    const Core::Game * game = Application::instance().gameCollection().findGame(m_game_id);
    if(!game)
        return false;
    mp_label_title->setText(game->title());
    QSettings settings;
    QString iso_dir = settings.value(SettingsKey::iso_recover_dir).toString();
    QString iso_filename = iso_dir.isEmpty() ?  game->title() + ".iso" :
        QDir(iso_dir).absoluteFilePath(game->title() + ".iso");
    iso_filename = QFileDialog::getSaveFileName(this, tr("Choose an ISO image filename to save"),
        iso_filename, tr("ISO Image") + " (*.iso)");
    if(iso_filename.isEmpty())
        return false;
    settings.setValue(SettingsKey::iso_recover_dir, QFileInfo(iso_filename).absoluteDir().absolutePath());
    restore(*game, iso_filename);
    return true;
}

void IsoRestorerWidget::restore(const Core::Game & _game, const QString & _destination)
{
    if(mp_working_thread) return;
    m_finish_status.clear();
    Core::IsoRestorer * restorer = new Core::IsoRestorer(_game, Application::instance().gameCollection().directory(), _destination, this);
    LambdaThread * working_thread = new LambdaThread([restorer]() {
        restorer->restore();
    }, this);
    mp_working_thread = working_thread;
    auto cleanup = [this, restorer]() {
        if(mp_working_thread)
        {
            mp_working_thread->deleteLater();
            mp_working_thread = nullptr;
            restorer->deleteLater();
        }
    };
    connect(mp_working_thread, &QThread::finished, this, &IsoRestorerWidget::onThreadFinished);
    connect(mp_working_thread, &QThread::finished, cleanup);
    connect(working_thread, &LambdaThread::exception, this, &IsoRestorerWidget::onException);
    connect(working_thread, &LambdaThread::exception, cleanup);
    connect(restorer, &Core::IsoRestorer::progress, this, &IsoRestorerWidget::onProgress);
    connect(restorer, &Core::IsoRestorer::rollbackStarted, this, &IsoRestorerWidget::onRollbackStarted);
    mp_progress_bar->setMinimum(0);
    mp_progress_bar->setMaximum(s_progress_max);
    mp_label_status->setText(tr("Restoring '%1' to '%2'...").arg(_game.title()).arg(_destination));
    mp_working_thread->start(QThread::HighestPriority);
}

void IsoRestorerWidget::onProgress(quint64 _total_bytes, quint64 _processed_bytes)
{
    if(_total_bytes == _processed_bytes)
    {
        mp_label_status->setText(tr("Synchronization of buffers. Please wait..."));
        m_finish_status = tr("Done");
        mp_button_box->setDisabled(true);
        mp_progress_bar->setMaximum(0);
        mp_progress_bar->setValue(0);
    }
    else
    {
        double progress = floor((static_cast<double>(_processed_bytes) / _total_bytes) * s_progress_max);
        mp_progress_bar->setValue(progress);
    }
}

void IsoRestorerWidget::onRollbackStarted()
{
    mp_progress_bar->setMaximum(0);
    mp_progress_bar->setValue(0);
    mp_label_status->setText(tr("Rolling back..."));
    m_finish_status = tr("Canceled");
}

void IsoRestorerWidget::onException(QString _message)
{
    mp_label_status->setText(_message);
    Application::instance().showErrorMessage(_message);
}

void IsoRestorerWidget::onThreadFinished()
{
    if(!m_finish_status.isEmpty())
        mp_label_status->setText(m_finish_status);
    mp_progress_bar->setMaximum(s_progress_max);
    mp_progress_bar->setValue(s_progress_max);
    mp_btn_back->setEnabled(true);
    mp_button_box->setDisabled(true);
}


void IsoRestorerWidget::onCancel()
{
    if(mp_working_thread)
    {
        mp_button_box->setDisabled(true);
        mp_working_thread->requestInterruption();
    }
}

