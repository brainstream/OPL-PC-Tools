/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/UI/GameImporterActivity.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/ChooseImportGamesDialog.h>
#include <OplPcTools/UlConfigGameStorage.h>
#include <QSettings>
#include <QFileDialog>
#include <QThread>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {
    const char * import_dir = "ImportDirectory";
} // namespace SettingsKey

const int g_progress_total = 10000;

class GameImporterIntent : public Intent
{
public:
    explicit GameImporterIntent(GameArtManager & _art_manager) :
        mr_art_manager(_art_manager)
    {
    }

    Activity * createActivity(QWidget * _parent)
    {
        return new GameImporterActivity(mr_art_manager, _parent);
    }

    QString activityClass() const
    {
        return "GameImporter";
    }

private:
    GameArtManager & mr_art_manager;
};

} // namespace name

class GameImporterActivity::WorkerThread : public QThread
{
public:
    WorkerThread(QObject * _parent) :
        QThread(_parent)
    {
    }

    void setImporters(QList<GameImporter *> & _importers)
    {
        m_importers = _importers;
    }

protected:
    void run() override
    {
        foreach(GameImporter * importer, m_importers)
            importer->import();
        m_importers = QList<GameImporter *>();
    }

private:
    QList<GameImporter *> m_importers;
};

GameImporterActivity::GameImporterActivity(GameArtManager & _art_manager, QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mr_art_manager(_art_manager),
    mp_thread(new WorkerThread(this)),
    m_total_count(0),
    m_done_count(0)
{
    setupUi(this);
    mp_progress_bar_overall->setMaximum(g_progress_total);
    mp_progress_bar_current->setMaximum(g_progress_total);
    mp_panel_progress->setVisible(false);
    connect(mp_button_box, &QDialogButtonBox::rejected, mp_thread, &QThread::requestInterruption);
    connect(mp_btn_close, &QPushButton::clicked, this, &QObject::deleteLater);
    connect(mp_thread, &QThread::finished, this, &GameImporterActivity::onThreadFinished);
}

bool GameImporterActivity::onAttach()
{
    QSettings settings;
    QString filter("ul.cfg");
    QString ul_dir = settings.value(SettingsKey::import_dir).toString();
    QString file = QFileDialog::getOpenFileName(this, tr("Select OPL Game Storage"), ul_dir, filter);
    if(file.isEmpty())
        return false;
    const QString source_directory = QFileInfo(file).absolutePath();
    QSharedPointer<GameCollection> source_collection(new GameCollection());
    source_collection->load(source_directory);
    if(!source_collection->isLoaded())
    {
        Application::showErrorMessage(tr("Unable to open game storage"));
        return false;
    }
    settings.setValue(SettingsKey::import_dir, source_directory);
    ChooseImportGamesDialog dlg(*source_collection, this);
    if(dlg.exec() != QDialog::Accepted)
    {
        return false;
    }
    QList<GameImporter *> importers;
    m_total_count = dlg.selectedGameIds().count();
    importers.reserve(m_total_count);
    foreach(const Uuid & id, dlg.selectedGameIds())
    {
        const Game * game = source_collection->findGame(id);
        if(!game)
        {
            Application::showErrorMessage(tr("Unable to read game from the storage"));
            return false;
        }
        GameImporter * importer = new GameImporter(source_collection, mr_art_manager, *game, this);
        importers.append(importer);
        connect(importer, &GameImporter::progress, this, &GameImporterActivity::onInstallerProgress);
        connect(importer, &GameImporter::error, this, &GameImporterActivity::onInstallerError);
        connect(importer, &GameImporter::rollbackStarted, this, &GameImporterActivity::onInstallerRollbackStarted);
        connect(importer, &GameImporter::rollbackFinished, this, &GameImporterActivity::onInstallerRollbackFinished);
    }
    mp_panel_progress->setVisible(true);
    setBusyUIState(true);
    mp_thread->setImporters(importers);
    mp_thread->start();
    return true;
}

void GameImporterActivity::setBusyUIState(bool _is_busy)
{
    mp_btn_close->setDisabled(_is_busy);
    mp_button_box->button(QDialogButtonBox::Cancel)->setEnabled(_is_busy);
}

void GameImporterActivity::onThreadFinished()
{
    setBusyUIState(false);
    Application::showMessage(tr("Done"), tr("Import complete"));
}

void GameImporterActivity::onInstallerProgress(const GameImportPorgress & _progress)
{
    switch(_progress.state)
    {
    case GameImportPorgress::State::DataCopying:
        setProgress(&_progress);
        break;
    case GameImportPorgress::State::ArtsCopying:
        setUnknownCurrentProgress(tr("Copying pictures..."));
        break;
    case GameImportPorgress::State::Done:
        ++m_done_count;
        setProgress(nullptr);
        break;
    default:
        break;
    }
}

void GameImporterActivity::setProgress(const GameImportPorgress * _current_progress)
{
    const float overall_part_for_game = static_cast<float>(g_progress_total) / m_total_count;
    float game_progress = 0;
    mp_progress_bar_current->setMaximum(g_progress_total);
    if(_current_progress)
    {
        game_progress = static_cast<float>(_current_progress->done_parts_bytes) / _current_progress->total_parts_bytes;
        mp_label_current->setText(tr("Importing '%1'...").arg(_current_progress->game_title));
        mp_progress_bar_current->setValue(static_cast<int>(g_progress_total * game_progress));
    }
    else
    {
        mp_label_current->setText(QString());
        mp_progress_bar_current->setValue(g_progress_total);
    }
    mp_progress_bar_overall->setValue(
        static_cast<int>(overall_part_for_game * m_done_count + overall_part_for_game * game_progress));
}

void GameImporterActivity::setUnknownCurrentProgress(const QString & _message)
{
    mp_progress_bar_current->setValue(0);
    mp_progress_bar_current->setMaximum(0);
    mp_label_current->setText(_message);
}

void GameImporterActivity::onInstallerError(const QString & _message)
{
    Application::showErrorMessage(_message);
}

void GameImporterActivity::onInstallerRollbackStarted()
{
    setUnknownCurrentProgress(tr("Rolling back..."));
}

void GameImporterActivity::onInstallerRollbackFinished()
{
    mp_progress_bar_current->setValue(0);
    mp_progress_bar_current->setMaximum(100);
    mp_label_current->setText(QString());
}

QSharedPointer<Intent> GameImporterActivity::createIntent(GameArtManager & _art_manager)
{
    return QSharedPointer<Intent>(new GameImporterIntent(_art_manager));
}
