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

#include <QDebug>
#include <OplPcTools/UI/IsoRestorerWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class IsoRestorerWidgetIntent : public Intent
{
public:
    explicit IsoRestorerWidgetIntent(const Core::Game & _game) :
        mr_game(_game)
    {
    }

    QWidget * createWidget(QWidget * _parent) override
    {
        IsoRestorerWidget * widget = new IsoRestorerWidget(_parent);
        widget->restore(mr_game);
        return widget;
    }

private:
    const Core::Game & mr_game;
    const QString m_destination;
};

} // namespace

IsoRestorerWidget::IsoRestorerWidget(QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mp_ui(new Ui::IsoRestorerWidget)
{
    mp_ui->setupUi(this);
    connect(mp_ui->mp_button_box, &QDialogButtonBox::rejected, this, &IsoRestorerWidget::deleteLater);
}

IsoRestorerWidget::~IsoRestorerWidget()
{
    delete mp_ui;
}

QSharedPointer<Intent> IsoRestorerWidget::createIntent(const Core::Game & _game)
{
    return QSharedPointer<Intent>(new IsoRestorerWidgetIntent(_game));
}

void IsoRestorerWidget::restore(const Core::Game & _game)
{
    qDebug() << "resotoring " << _game.title() << "...";
}
