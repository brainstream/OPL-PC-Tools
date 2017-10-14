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

#ifndef __OPLPCTOOLS_MAINWINDOW__
#define __OPLPCTOOLS_MAINWINDOW__

#include <QLabel>
#include <OplPcTools/Core/GameCollection.h>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent * _event) override;

private slots:
    void about();
    void aboutQt();
    void showSettings();
    void loadGameCollection();
    void reloadUlConfig();
    void renameGame();
    void addGame();
    void gameToIso();
    void deleteGame();
    void setCover();
    void removeCover();
    void setIcon();
    void removeIcon();
    void manageArts();
    void gameSelectionChanged();
    void gameInstalled(const QString & _id);

private:
    void initUi();
    void setupChangePixmapButton(QToolButton * _btn, const char * _remove_slot);
    QString getOpenPicturePath(const QString & _title);
    void loadGameCollection(const QDir & _directory);
    void setCurrentFilePath(const QString & _path);
    void activateFileActions(bool _activate);
    void activateGameActions(const Game * _selected_game);

private:
    QLabel * mp_label_current_root;
    QPixmap * mp_no_image;
    GameCollection m_game_collection;
};

#endif // __OPLPCTOOLS_MAINWINDOW__
