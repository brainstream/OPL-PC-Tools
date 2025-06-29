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
#include <OplPcTools/UI/ChooseUlGamesDialog.h>
#include <OplPcTools/UlConfigGameStorage.h>
#include <OplPcTools/GameImporter.h>
#include <QSettings>
#include <QFileDialog>
#include <QThread>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {
    const char * import_dir = "ImportDirectory";
} // namespace SettingsKey

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
    mp_thread(new WorkerThread(this))
{
    setupUi(this);
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
    std::unique_ptr<UlConfigGameStorage> storage(new UlConfigGameStorage(this));
    if(!storage->load(source_directory))
    {
        Application::showErrorMessage(tr("Unable to open game storage"));
        return false;
    }
    settings.setValue(SettingsKey::import_dir, source_directory);
    ChooseUlGamesDialog dlg(*storage, this);
    if(dlg.exec() != QDialog::Accepted)
    {
        return false;
    }
    QList<GameImporter *> importers;
    importers.reserve(dlg.selectedGameIds().count());
    foreach(const Uuid & id, dlg.selectedGameIds())
    {
        const Game * game = storage->findGame(id);
        if(!game)
        {
            Application::showErrorMessage(tr("Unable to read game from the storage"));
            return false;
        }
        importers.append(new GameImporter(source_directory, mr_art_manager, *game, this));
    }
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
}

QSharedPointer<Intent> GameImporterActivity::createIntent(GameArtManager & _art_manager)
{
    return QSharedPointer<Intent>(new GameImporterIntent(_art_manager));
}
