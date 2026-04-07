#ifndef PARSER_H
#define PARSER_H

#include "fortstruct.h"

#include <QString>

namespace Forting
{

// class Parser {
// private:
//     QString command;
//     std::vector<FileEntry>* fileList;
//     std::vector<FortingClass>* tagList;
// protected:
//     static const FileEntry* fileptr;
//     static QStringList commandList;
//     static int currentLine;
//     static int currentLayer;
//     static QString trim(QString s);
//     static QString getAword(QString& s);
//     static unit unit_parsing(const QString& s);
//     static unit unit_parsing(const QString& s, Attri objAttri);
//     static bool booleanUnit_parsing(const QString& s);
// public:
//     Parser() { currentLine=0;fileptr=nullptr;fileList=nullptr;tagList=nullptr;
//                 command="";}
//     bool changePtr(const FileEntry& fe);
//     void linkFileList(std::vector<FileEntry>& fl) {fileList = &fl;}
//     void linkTagList(std::vector<FortingClass>& tl) {tagList = &tl;}
//     bool begin(QString& errorMsg);
// };

// class ifelse: Parser {
// private:
//     std::vector<Expr> exprlist;
//     std::vector<Expr> actionlist;
//     int beginLine;
//     int endLine;
// public:
//     bool begin(QString& errorMsg);
// };

// class when: Parser {
// private:
// };

// class booleanExpr: Parser {
//     bool ok;
//     bool result;
//     booleanExpr();
//     bool compare();
// };


}

#endif // PARSER_H
