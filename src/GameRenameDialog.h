#ifndef GAMERENAMEDIALOG_H
#define GAMERENAMEDIALOG_H

#include "ui_GameRenameDialog.h"

class GameRenameDialog : public QDialog, private Ui::GameRenameDialog
{
    Q_OBJECT

public:
    explicit GameRenameDialog(const QString & _initial_name, QWidget *_parent = nullptr);
    QString name() const;

private slots:
    void nameChanged(const QString & _name);
};

#endif // GAMERENAMEDIALOG_H
