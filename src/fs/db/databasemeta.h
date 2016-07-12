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

#ifndef ATOOLS_FS_DB_DATABASEMETA_H
#define ATOOLS_FS_DB_DATABASEMETA_H

#include <QDateTime>

namespace atools {
namespace sql {
class SqlDatabase;
}
namespace fs {
namespace db {

class DatabaseMeta
{
public:
  DatabaseMeta(atools::sql::SqlDatabase *sqlDb);

  int getMajorVersion() const
  {
    return majorVersion;
  }

  int getMinorVersion() const
  {
    return minorVersion;
  }

  QDateTime getLastLoadTime() const
  {
    return lastLoadTime;
  }

  bool isValid() const
  {
    return valid;
  }

  bool hasSchema();
  bool hasData();
  bool isDatabaseCompatible();
  bool isDatabaseCompatible(int majorVersion);

  void updateVersion(int majorVer, int minorVer);
  void updateVersion();
  void updateTimestamp();
  void updateAll();

  /* This defines the database schema version and should be updated for every incompatible
   * schema or content change */
  static Q_DECL_CONSTEXPR int DB_VERSION_MAJOR = 2;

  /* History:
   * 1 Removed unused database fields light_flags and pattern_flags.
   * 2 Increased ILS feather length to 9 nm
   * 3 Removed taxi path weight limit
   */
  static Q_DECL_CONSTEXPR int DB_VERSION_MINOR = 3;

private:
  atools::sql::SqlDatabase *db;
  int majorVersion = 0, minorVersion = 0;
  QDateTime lastLoadTime;
  bool valid = false;
};

} // namespace db
} // namespace fs
} // namespace atools

#endif // ATOOLS_FS_DB_DATABASEMETA_H