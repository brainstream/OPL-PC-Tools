#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include "ui_MainWindow.h"

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
    void gameSelected(QListWidgetItem * _item);

private:
    void loadUlConfig(const QString & _filename);
    void setCurrentFilePath(const QString & _path);
    void activateFileActions(bool _activate);
    void activateGameActions(bool _activate);

private:
    QLabel * mp_label_current_ul_file;
};

#endif // MAINWINDOW_H
