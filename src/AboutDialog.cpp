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

#include <QtGlobal>
#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    QApplication * app = static_cast<QApplication *>(QApplication::instance());
    mp_label_app_name->setText(app->applicationDisplayName());
    static const int start_development_year = 2017;
    int build_year = QString(__DATE__).right(4).toInt();
    QString years = (start_development_year < build_year) ?
        QString("%1 - %2").arg(start_development_year).arg(build_year) :
        QString::number(build_year);
    mp_label_version->setText(mp_label_version->text().arg(QT_STRINGIFY(_OPLPCTOOLS_VERSION)));
    mp_label_description->setText(mp_label_description->text().arg(years));
}
