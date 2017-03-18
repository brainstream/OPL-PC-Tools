#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QListWidgetItem>
#include <cdio/cdio.h>
#include <cdio/iso9660.h>
#include "MainWindow.h"
#include "UlConfig.h"
#include "GameRenameDialog.h"
#include "Exception.h"

namespace {

static const char * g_settings_key_wnd_geometry = "window geometry";
static const char * g_settings_key_ul_dir = "uldir";

class GameListItem : public QListWidgetItem
{
public:
    explicit GameListItem(Ul::ConfigRecord _config_record) :
        m_config_record(_config_record)
    {
    }

    QVariant data(int _role) const;

    Ul::ConfigRecord & configRecord()
    {
        return m_config_record;
    }

private:
    Ul::ConfigRecord m_config_record;
};

QVariant GameListItem::data(int _role) const
{
    if(_role == Qt::DisplayRole)
        return m_config_record.name;
    return QListWidgetItem::data(_role);
}

} // namespace

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    mp_widget_game_details->setVisible(false);
    mp_label_current_ul_file = new QLabel(mp_statusbar);
    mp_statusbar->addWidget(mp_label_current_ul_file);
    activateFileActions(false);
    activateGameActions(false);
    QSettings settings;
    restoreGeometry(settings.value(g_settings_key_wnd_geometry).toByteArray());
}

void MainWindow::closeEvent(QCloseEvent * _event)
{
    QSettings settings;
    settings.setValue(g_settings_key_wnd_geometry, saveGeometry());
}

void MainWindow::loadUlConfig()
{
    QSettings settings;
    QString dirpath = settings.value(g_settings_key_ul_dir).toString();
    dirpath = QFileDialog::getExistingDirectory(this, tr("Choose the OPL root dir"), dirpath);
    if(dirpath.isEmpty()) return;
    settings.setValue(g_settings_key_ul_dir, dirpath);
    QDir dir(dirpath);
    QString ul_file = dir.filePath("ul.cfg");
    loadUlConfig(ul_file);
}

void MainWindow::loadUlConfig(const QString & _filename)
{
    mp_list_games->clear();
    if(!QFile(_filename).exists())
    {
        mp_label_current_ul_file->setText(_filename);
        activateFileActions(true);
        return;
    }
    try
    {
        QList<Ul::ConfigRecord> config = Ul::loadConfig(_filename);
        for(const auto & record : config)
            mp_list_games->addItem(new GameListItem(record));
        mp_label_current_ul_file->setText(_filename);
        mp_list_games->setCurrentRow(0);
        activateFileActions(true);
    }
    catch(const Exception & exception)
    {
        mp_label_current_ul_file->clear();
        activateFileActions(false);
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::reloadUlConfig()
{
    loadUlConfig(mp_label_current_ul_file->text());
}

void MainWindow::activateFileActions(bool _activate)
{
    mp_action_add_game->setEnabled(_activate);
    mp_action_reload_file->setEnabled(_activate);
}

void MainWindow::activateGameActions(bool _activate)
{
    mp_action_delete_game->setEnabled(_activate);
    mp_action_rename_game->setEnabled(_activate);
}

void MainWindow::gameSelected(QListWidgetItem * _item)
{
    if(_item == nullptr)
    {
        mp_widget_game_details->setVisible(false);
        activateGameActions(false);
        return;
    }
    GameListItem * item = static_cast<GameListItem *>(_item);
    const Ul::ConfigRecord & record = item->configRecord();
    mp_label_game_id->setText(record.image);
    mp_label_game_title->setText(record.name);
    mp_label_game_parts->setText(QString("%1").arg(record.parts));
    switch(record.type)
    {
    case MediaType::cd:
        mp_label_game_type->setText("CD");
        break;
    case MediaType::dvd:
        mp_label_game_type->setText("DVD");
        break;
    default:
        mp_label_game_type->setText(tr("Unknown"));
        break;
    }
    mp_widget_game_details->setVisible(true);
    activateGameActions(true);
}

void MainWindow::renameGame()
{
    GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
    if(item == nullptr) return;
    Ul::ConfigRecord & config_record = item->configRecord();
    try
    {
        GameRenameDialog dlg(config_record.name, this);
        if(dlg.exec() == QDialog::Accepted)
        {
            QString new_name = dlg.name();
            Ul::renameConfigRecord(config_record.image, new_name, mp_label_current_ul_file->text());
            config_record.name = new_name;
            mp_list_games->update(mp_list_games->currentIndex());
            gameSelected(item);
        }
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

/*
void testISO()
{
    cdio_init();
    iso9660_t * iso = iso9660_open("/home/brainstream/downloads/kubuntu-16.10-desktop-amd64.iso");
//    iso9660_t * iso = iso9660_open("/raid/games/PS2/SLES_544.90.GH [PS2 PAL MULTI5].iso");
    iso9660_pvd_t pvd;
    iso9660_ifs_read_pvd(iso, &pvd);
//    mp_out->setPlainText(pvd.volume_id);
    iso9660_close(iso);
}

void testCD()
{
    cdio_init();
    CdIo_t * cdio = cdio_open_cd("/dev/sr0");
    track_t first_track = cdio_get_first_track_num(cdio);
    cdtext_t * text = cdio_get_cdtext(cdio, first_track);
    QString out_text;
    for(char i = 0; i <= 12; ++i)
    {
        char * title = cdtext_get((cdtext_field_t)i, text);
        out_text += title;
        out_text += '\n';
        free(title);
    }
//    mp_out->setPlainText(out_text);
    cdio_destroy(cdio);
}
*/
