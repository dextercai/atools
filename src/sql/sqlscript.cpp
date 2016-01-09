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

#include "sql/sqlexception.h"
#include "sql/sqlscript.h"
#include "logging/loggingdefs.h"

#include <QFile>
#include <QTextStream>

namespace atools {

namespace sql {

SqlScript::SqlScript(SqlDatabase *sqlDb)
  : db(sqlDb)
{
}

void SqlScript::executeScript(const QString& filename)
{
  QFile scriptFile(filename);
  if(scriptFile.open(QIODevice::Text | QIODevice::ReadOnly))
  {
    QTextStream scriptStream(&scriptFile);

    qInfo() << "-- Running script ------------------------------------------";
    qInfo() << "--" << scriptFile.fileName() << "--";
    executeScript(scriptStream);
  }
  else
    throw SqlException(
            QString("Cannot open script file \"%1\". Reason: %2.").
            arg(scriptFile.fileName()).arg(scriptFile.errorString()));
  scriptFile.close();
}

void SqlScript::executeScript(QTextStream& script)
{
  QList<ScriptCmd> statements;
  parseSqlScript(script, statements);

  SqlQuery query(db);
  for(ScriptCmd cmd : statements)
  {
    qInfo().nospace() << cmd.lineNumber << ": " << cmd.sql;
    query.exec(cmd.sql);
    qInfo().nospace() << "[" << query.numRowsAffected() << "]";
  }

  qInfo() << "-- Done Running script ------------------------------------------";
}

void SqlScript::parseSqlScript(QTextStream& script, QList<ScriptCmd>& statements)
{
  QString line;
  QString currentStatement;
  bool isSingleString = false, isDoubleString = false, isBlockComment = false;
  int currentLine = 1;

  while(!script.atEnd())
  {
    QChar lastChar = 0;

    line = script.readLine();

    if(currentStatement.size() > 1)
      // Already appending to a statement -
      // so substitute the line end with a space
      currentStatement += " ";

    for(QChar currentChar : line)
    {
      if(currentChar == '-')
      {
        if(!isSingleString && !isDoubleString && !isBlockComment && lastChar == '-')
        {
          // Start of line comment: "--" - remove character from statement
          currentStatement.chop(1);
          // Stop reading this line
          break;
        }
        else if(!isBlockComment)
          // no comment at all - append character to statement
          currentStatement.push_back(currentChar);
      }
      else if(currentChar == '/')
      {
        if(!isSingleString && !isDoubleString && isBlockComment && lastChar == '*')
          // End of block comment
          isBlockComment = false;
        else if(!isBlockComment)
          // no comment at all - append character to statement
          currentStatement.push_back(currentChar);
      }
      else if(currentChar == '*')
      {
        if(!isSingleString && !isDoubleString && lastChar == '/')
        {
          // Start of block comment - remove character from statement
          currentStatement.chop(1);
          isBlockComment = true;
        }
        else if(!isBlockComment)
          // no comment at all - append character to statement
          currentStatement.push_back(currentChar);
      }
      else if(currentChar == '\'')
      {
        // Recognize string literals and ignore any comment character within
        if(!isBlockComment && !isDoubleString)
        {
          isSingleString = !isSingleString;
          currentStatement.push_back(currentChar);
        }
        else if(isDoubleString)
          currentStatement.push_back(currentChar);
      }
      else if(currentChar == '"')
      {
        // Recognize string literals and ignore any comment character within
        if(!isBlockComment && !isSingleString)
        {
          isDoubleString = !isDoubleString;
          currentStatement.push_back(currentChar);
        }
        else if(isSingleString)
          currentStatement.push_back(currentChar);
      }
      else if(currentChar == ';')
      {
        if(!isSingleString && !isDoubleString)
        {
          if(!isBlockComment)
          {
            // End of statement - reset all values
            statements.push_back(ScriptCmd({currentStatement, currentLine}));
            currentStatement.clear();
            isSingleString = false;
            isDoubleString = false;
            isBlockComment = false;
            lastChar = 0;
          }
        }
        else
          // A ";" in a string literal - append
          currentStatement.push_back(currentChar);
      }
      else if(!isBlockComment)
      {
        if(isSingleString || isDoubleString)
          // Add string literally
          currentStatement.push_back(currentChar);
        else if(!(currentChar == ' ' && lastChar == ' '))
          if(!(currentChar == ' ' && currentStatement.size() == 0))
            // Skip double and leading spaces
            currentStatement.push_back(currentChar);
      }
      lastChar = currentChar;
    }
    currentLine++;
  }
}

} // namespace sql

} // namespace atools