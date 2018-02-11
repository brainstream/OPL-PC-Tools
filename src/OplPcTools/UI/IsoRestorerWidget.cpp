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
#include <QFileDialog>
#include <QSettings>
#include <OplPcTools/Core/Game.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/IsoRestorerWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {

static const char * iso_recover_dir = "ISORecoverDirectory";

} // namespace SettingsKey

class IsoRestorerWidgetIntent : public Intent
{
public:
    explicit IsoRestorerWidgetIntent(const QString & _game_id) :
        m_game_id(_game_id)
    {
    }

    Activity * createActivity(QWidget * _parent) override
    {
        IsoRestorerWidget * widget = new IsoRestorerWidget(m_game_id, _parent);
        return widget;
    }

private:
    const QString m_game_id;
};

} // namespace

IsoRestorerWidget::IsoRestorerWidget(const QString & _game_id, QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    m_game_id(_game_id)
{
    setupUi(this);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &IsoRestorerWidget::deleteLater);
}

QSharedPointer<Intent> IsoRestorerWidget::createIntent(const QString & _game_id)
{
    return QSharedPointer<Intent>(new IsoRestorerWidgetIntent(_game_id));
}

bool IsoRestorerWidget::onAttach()
{
    const Core::Game * game = Application::instance().gameCollection().findGame(m_game_id);
    if(!game)
        return false;
    QSettings settings;
    QString iso_dir = settings.value(SettingsKey::iso_recover_dir).toString();
    QString iso_filename = iso_dir.isEmpty() ?  game->title() + ".iso" :
        QDir(iso_dir).absoluteFilePath(game->title() + ".iso");
    iso_filename = QFileDialog::getSaveFileName(this, tr("Choose an ISO image filename to save"),
        iso_filename, tr("ISO Image") + " (*.iso)");
    if(iso_filename.isEmpty())
        return false;
    settings.setValue(SettingsKey::iso_recover_dir, QFileInfo(iso_filename).absoluteDir().absolutePath());
    restore(*game, iso_filename);
    return true;
}

void IsoRestorerWidget::restore(const Core::Game & _game, const QString & _destination)
{
    qDebug() << "resotoring" << _game.title() << "to" << _destination;
}
