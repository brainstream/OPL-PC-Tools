#include <QPushButton>
#include "GameRenameDialog.h"
#include "UlConfig.h"

GameRenameDialog::GameRenameDialog(const QString & _initial_name, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    mp_edit_name->setText(_initial_name);
    mp_edit_name->selectAll();
}

QString GameRenameDialog::name() const
{
    return mp_edit_name->text();
}

void GameRenameDialog::nameChanged(const QString & _name)
{
    int bytes_left = UL_MAX_GAME_NAME_LENGTH - _name.toUtf8().length();
    if(bytes_left < 0)
    {
        mp_label_bytes->setText(tr("Length exceeded by %1 byte(s)").arg(-bytes_left));
        mp_button_box->button(QDialogButtonBox::Ok)->setDisabled(true);
    }
    else
    {
        mp_label_bytes->setText(tr("%1 byte(s) left").arg(bytes_left));
        mp_button_box->button(QDialogButtonBox::Ok)->setDisabled(_name.isEmpty());
    }
}
