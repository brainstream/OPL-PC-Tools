/***********************************************************************************************
 *                                                                                             *
 * This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.        *
 *                                                                                             *
 * qpcopl is free software: you can redistribute it and/or modify it under the terms of        *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;        *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#ifndef __QPCOPL_MAINWINDOW__
#define __QPCOPL_MAINWINDOW__

#include <QLabel>
#include "ui_MainWindow.h"
#include "UlConfig.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent * _event) override;

private slots:
    void loadUlConfig();
    void reloadUlConfig();
    void renameGame();
    void addGame();
    void deleteGame();
    void gameSelected(QListWidgetItem * _item);

private:
    void loadUlConfig(const QDir & _directory);
    void setCurrentFilePath(const QString & _path);
    void activateFileActions(bool _activate);
    void activateGameActions(bool _activate);

private:
    QLabel * mp_label_current_ul_file;
    QSharedPointer<UlConfig> m_config_ptr;
};

#endif // __QPCOPL_MAINWINDOW__
