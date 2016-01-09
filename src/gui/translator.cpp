/*****************************************************************************
* Copyright 2015-2016 Alexander Barthel albar965@mailbox.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "gui/translator.h"
#include "logging/loggingdefs.h"

#include <QFileInfo>
#include <QCoreApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

namespace atools {

namespace gui {

QVector<QTranslator *> Translator::translators;

void Translator::load(const QString& language)
{
  if(translators.isEmpty())
  {
    QFileInfo appFilePath(QCoreApplication::applicationFilePath());
    QString appPath = appFilePath.absolutePath();
    QString appBaseName = appFilePath.baseName();

    // try resources first
    if(!loadAndInstall(appBaseName, ":/" + appBaseName, language))
      // try resources translations second
      if(!loadAndInstall(appBaseName, ":/" + appBaseName + "/translations", language))
        // Last try in executable directory
        loadAndInstall(appBaseName, appPath, language);

    QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    // First application path
    if(!loadAndInstall("qt", appPath, language))
      // second official translations path
      loadAndInstall("qt", translationsPath, language);

    if(!loadAndInstall("qtbase", appPath, language))
      loadAndInstall("qtbase", translationsPath, language);
  }
  else
    qWarning() << "Translator::load called more than once";
}

void Translator::unload()
{
  if(!translators.isEmpty())
  {
    qDeleteAll(translators);
    translators.clear();
  }
  else
    qWarning() << "Translator::unload called more than once";
}

bool Translator::loadAndInstall(const QString& name, const QString& dir, const QString& language)
{
  QLocale locale;
  if(language.isEmpty())
    locale = QLocale::system();
  else
    // Override system language for translations only
    locale = QLocale(language);

  QTranslator *t = new QTranslator();
  if(!t->load(locale, name, "_", dir))
    qWarning() << "Qt translation file" << name << "not loaded from dir" << dir;
  else if(QCoreApplication::instance()->installTranslator(t))
  {
    qDebug() << "Qt translation file" << name << "from dir" << dir << "installed";
    translators.append(t);
    return true;
  }
  else
    qDebug() << "Qt translation file" << name << "not installed from dir" << dir;

  delete t;
  return false;
}

} // namespace gui
} // namespace atools