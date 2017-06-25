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

#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    QApplication * app = static_cast<QApplication *>(QApplication::instance());
    mp_label_app_name->setText(app->applicationDisplayName());
    static const int start_development_year = 2017;
    QString years = (start_development_year < _QPCOPL_BUIDL_YEAR) ?
        QString("%1 - %2").arg(start_development_year).arg(_QPCOPL_BUIDL_YEAR) :
        QString::number(_QPCOPL_BUIDL_YEAR);
    mp_label_version->setText(mp_label_version->text().arg(QT_STRINGIFY(_QPCOPL_VERSION)));
    mp_label_description->setText(mp_label_description->text().arg(years));
}
