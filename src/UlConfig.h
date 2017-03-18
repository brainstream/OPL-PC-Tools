#ifndef ULCONFIG_H
#define ULCONFIG_H

#include <QString>
#include <QList>

#define UL_MAX_GAME_NAME_LENGTH 32

enum class MediaType
{
    unknown,
    cd,
    dvd
};

namespace Ul {

struct ConfigRecord
{
    QString name;
    QString image;
    MediaType type;
    quint8 parts;
};

QList<ConfigRecord> loadConfig(const QString & _filepath);
void addConfigRecord(const ConfigRecord & _config, const QString & _filepath);
void deleteConfigRecord(const QString _image, const QString & _filepath);
void renameConfigRecord(const QString _image, const QString & _new_name, const QString & _filepath);

} // namespace Ul

#endif // ULCONFIG_H
