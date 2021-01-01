/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#include <QIcon>
#include <QDir>
#include <QSet>
#include <QSettings>
#include <OplPcTools/UI/IconTheme.h>

namespace {

QString tryReadIconTheme(const QDir _dir)
{
    QString filename = _dir.absoluteFilePath("index.theme");
    if(!QFile::exists(filename))
        return QString();
    QSettings settings(filename, QSettings::IniFormat);
    return settings.value("Icon Theme/Name", QString()).toString();
}

QStringList scanIconThemes(const QDir & _dir)
{
    QStringList result;
    if(!_dir.exists())
        return result;
    for(const QString & theme_dir : _dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString theme = tryReadIconTheme(_dir.absoluteFilePath(theme_dir));
        if(!theme.isEmpty())
            result << theme;
    }
    return result;
}

} // namespace

namespace OplPcTools {
namespace UI {

QStringList loadIconThemes()
{
    QSet<QString> theme_set;
    for(const QString & path : QIcon::themeSearchPaths())
    {
        for(const QString & theme : scanIconThemes(path))
            theme_set.insert(theme);
    }
    QStringList result(theme_set.begin(), theme_set.end());
    std::sort(result.begin(), result.end());
    return result;
}

} // namespace UI
} // namespace OplPcTools
