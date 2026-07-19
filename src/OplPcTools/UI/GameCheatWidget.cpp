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

#include <OplPcTools/UI/GameCheatWidget.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/BusySmartThread.h>
#include <OplPcTools/UI/ProgressDialog.h>
#include <OplPcTools/UI/AcceptCheatDialog.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/File.h>
#include <QToolTip>

using namespace OplPcTools;
using namespace OplPcTools::UI;

GameCheatWidget::GameCheatWidget(const Game & _game, QWidget * _parent) :
    QWidget(_parent),
    mp_cheat_manager(new GameCheatManager(Library::instance().directory())),
    m_game_id(_game.id())
{
    setupUi(this);
    if(std::optional<QString> cheat = loadCheat())
        mp_text_edit_cheat->setText(cheat.value());
    connect(mp_btn_save, &QPushButton::clicked, this, &GameCheatWidget::saveCheat);
    connect(mp_btn_delete, &QPushButton::clicked, this, &GameCheatWidget::deleteCheat);
    connect(mp_btn_download, &QPushButton::clicked, this, &GameCheatWidget::download);
}

GameCheatWidget::~GameCheatWidget()
{
    delete mp_cheat_manager;
}

std::optional<QString> GameCheatWidget::loadCheat()
{
    try
    {
        return mp_cheat_manager->load(m_game_id);
    }
    catch(const Exception & _exception)
    {
        Application::showErrorMessage(_exception.message());
    }
    catch(...)
    {
        Application::showErrorMessage();
    }
    return std::nullopt;
}

void GameCheatWidget::saveCheat()
{
    startSmartThread(
        [this]() {
            const QString text = mp_text_edit_cheat->toPlainText();
            mp_cheat_manager->save(m_game_id, text);
        },
        [this]() {
            QToolTip::showText(mapToGlobal(mp_btn_save->pos() - QPoint(0, 50)), tr("Cheat saved"), this, QRect(), 3000);
        });
}

void GameCheatWidget::deleteCheat()
{
    if(QMessageBox::Yes == QMessageBox::question(this, tr("Confirmation"),
        tr("Are you sure you want to delete the cheat?"),
        QMessageBox::Yes | QMessageBox::No))
    {
        startSmartThread(
            [this]() {
                mp_cheat_manager->remove(m_game_id);
            },
            [this]() {
                QTextCursor cursor = mp_text_edit_cheat->textCursor();
                cursor.select(QTextCursor::Document);
                cursor.removeSelectedText(); // Support for undo
            });
    }
}

void GameCheatWidget::download()
{
    const QString process_name = tr("Downloading cheat");
    ProgressDialog * progress_dialog = new ProgressDialog(this);
    progress_dialog->setWindowTitle(process_name);
    progress_dialog->setProgressLabelText(process_name + "...");
    progress_dialog->show();
    GameCheatDownloader * downloader = new GameCheatDownloader(m_game_id, this);

    connect(downloader, &GameCheatDownloader::finished, this, [progress_dialog, downloader] {
        downloader->deleteLater();
        progress_dialog->accept();
        progress_dialog->deleteLater();
    });
    connect(downloader, &GameCheatDownloader::downloaded, this, [this](const QString & __cheat) {
        mp_cheat_manager->save(m_game_id, __cheat);
        mp_text_edit_cheat->setText(__cheat);
    });
    connect(downloader, &GameCheatDownloader::error, this, [](const QString & __error) {
        Application::showMessage(__error);
    });
    connect(progress_dialog, &ProgressDialog::canceled, downloader, &GameCheatDownloader::cancel);
    downloader->start();
}

void GameCheatWidget::startSmartThread(std::function<void()> _action, std::function<void()> _finished)
{
    BusySmartThread * thread = new BusySmartThread(_action, nullptr, this);
    connect(thread, &BusySmartThread::finished, this, _finished);
    connect(thread, &BusySmartThread::finished, thread, &BusySmartThread::deleteLater);
    connect(thread, &BusySmartThread::exception, this, [](const QString & message) {
        Application::showErrorMessage(message);
    });
    thread->setSpinnerDisplayTimeout(300);
    thread->start();
}
