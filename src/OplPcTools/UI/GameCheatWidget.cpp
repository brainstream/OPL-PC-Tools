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
#include <OplPcTools/ApplicationInfo.h>
#include <QToolTip>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace OplPcTools;
using namespace OplPcTools::UI;

GameCheatWidget::GameCheatWidget(const Game & _game, QWidget * _parent) :
    QWidget(_parent),
    m_filename(GameCheat::makeFilename(Library::instance().directory(), _game.id()))
{
    loadCheat();
    setupUi(this);
    mp_text_edit_cheat->setText(m_cheat_ptr->text);
    connect(mp_btn_save, &QPushButton::clicked, this, &GameCheatWidget::saveCheat);
    connect(mp_btn_delete, &QPushButton::clicked, this, &GameCheatWidget::deleteCheat);
    connect(mp_btn_download, &QPushButton::clicked, this, &GameCheatWidget::download);
}

bool GameCheatWidget::loadCheat()
{
    try
    {
        m_cheat_ptr = GameCheat::load(m_filename);
        return true;
    }
    catch(const Exception & _exception)
    {
        Application::showErrorMessage(_exception.message());
    }
    catch(...)
    {
        Application::showErrorMessage();
    }
    return false;
}

void GameCheatWidget::saveCheat()
{
    startSmartThread(
        [this]() {
            m_cheat_ptr->text = mp_text_edit_cheat->toPlainText();
            GameCheat::save(*m_cheat_ptr, m_filename);
        },
        [this]() {
            QToolTip::showText(mapToGlobal(mp_btn_save->pos() - QPoint(0, 50)), tr("Cheat saved"), this, QRect(), 3000);
        });
}

void GameCheatWidget::deleteCheat()
{
    QFile cheat_file(m_filename);
    if(!cheat_file.exists())
    {
        QToolTip::showText(
            mapToGlobal(mp_btn_delete->pos() - QPoint(0, 50)),
            tr("Cheat file does not exist"),
            this,
            QRect(),
            3000);
        return;
    }
    if(QMessageBox::Yes == QMessageBox::question(this, tr("Confirmation"),
        tr("Are you sure you want to delete the cheat file?\n%1").arg(m_filename),
        QMessageBox::Yes | QMessageBox::No))
    {
        startSmartThread(
            [this]() {
                removeFile(m_filename);
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
    const QString cheat_name = QFileInfo(m_filename).fileName();
    const QString process_name = tr("Downloading cheat %1").arg(cheat_name);
    ProgressDialog * progress_dialog = new ProgressDialog(this);
    progress_dialog->setWindowTitle(process_name);
    progress_dialog->setProgressLabelText(process_name + "...");
    progress_dialog->show();
    QNetworkAccessManager * network = new QNetworkAccessManager(this);
    const QString url =
        QString("https://raw.githubusercontent.com/PS2-Widescreen/OPL-Widescreen-Cheats/refs/heads/main/CHT/%1")
        .arg(cheat_name);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, APPLICATION_DISPLAY_NAME);
    request.setTransferTimeout(std::chrono::seconds(15));
    QNetworkReply * reply = network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, progress_dialog, network, reply] {
        reply->deleteLater();
        network->deleteLater();
        progress_dialog->accept();
        progress_dialog->deleteLater();
        switch(reply->error())
        {
        case QNetworkReply::NoError:
        {
            QString text = reply->readAll();
            AcceptCheatDialog dlg(text, this);
            if(dlg.exec() == QDialog::Accepted)
            {
                mp_text_edit_cheat->setText(text);
                saveCheat();
            }
            break;
        }
        case QNetworkReply::ContentNotFoundError:
            Application::showMessage(tr("Cheat for this game not found"));
            break;
        default:
            Application::showErrorMessage(tr("Unable to download the cheat, a network error occurred"));
            break;
        }
    });
    connect(progress_dialog, &ProgressDialog::canceled, reply, &QNetworkReply::abort);
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
