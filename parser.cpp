#include "parser.h"

#include <QStringList>

/*
namespace Forting
{

bool Parser::changePtr(const FileEntry& fe) {
    if(fe.name==nullptr) return false;
    this->fileptr = &fe;
    return true;
}

unit Parser::unit_parsing(const QString& _s) {
    unit u;
    QString s = _s;
    do
    {
        QStringList parts = s.split('.');
        if(s=='>') { u.attri =  Attri::greater; }
        else if(s=='<') { u.attri = Attri::less; }
        else if(s=='=') { u.attri = Attri::equal; }
        else if(s==">=") { u.attri = Attri::greaterEqu; }
        else if(s=="<=") { u.attri = Attri::lessEqu; }
        else if(s=="!=") { u.attri = Attri::notEqu; }
        else if(parts[0]=="name") { u.attri = Attri::name;u.value=fileptr->name; }
        else if(parts[0]=="size") { u.attri = Attri::size;u.value=fileptr->size; }
        else if(parts[0]=="suffix") { u.attri = Attri::suffix;u.value=fileptr->suffix; }
        else if(parts[0]=="modified") {
            if(parts[1]=="year") u.attri = Attri::m_y;
            else if(parts[1]=="month") u.attri = Attri::m_m;
            else if(parts[1]=="day") u.attri = Attri::m_d;
            else if(parts[1]=="yearmonth") u.attri = Attri::m_ym;
            else if(parts[1]=="date") u.attri = Attri::m_ymd;
            else break;
            u.value=fileptr->modified;
        }
        else if(parts[0]=="created") {
            if(parts[1]=="year") u.attri = Attri::c_y;
            else if(parts[1]=="month") u.attri = Attri::c_m;
            else if(parts[1]=="day") u.attri = Attri::c_d;
            else if(parts[1]=="yearmonth") u.attri = Attri::c_ym;
            else if(parts[1]=="date") u.attri = Attri::m_ymd;
            else break;
            u.value=fileptr->created;
        }
        else break;
        return u;
    } while(0);
    u.attri = Attri::ERROR;
    u.value = -1;
    return u;
}

unit Parser::unit_parsing(const QString& s, Attri objAttri) {
    unit u;
    QStringList sl;
    switch (objAttri) {
    case Attri::name :
        sl = s.split("|");
        for(auto &part: sl) {

        }
    default :
        u.attri = Attri::ERROR;
        u.value = -1;
    }
}

QString Parser::trim(QString s) {
    while(!s.isEmpty() && s.front().isSpace()!=0) s.removeFirst();
    while(!s.isEmpty() && s.back().isSpace()!=0) s.removeLast();
}

QString Parser::getAword(QString& s) {
    QString r="";
    for(int i=0;i<s.length();++i) {
        if(!s[i].isSpace()) r.push_back(s.removeAt(i));
        else break;
    }
    return r;
}

bool Parser::booleanUnit_parsing(const QString& _s) {
    QString attri;


}

bool Parser::begin(QString& errorMsg) {
    if(!this->fileList) {
        if(this->fileList->empty()) errorMsg = "No File!";
        else errorMsg = "file list Not Linked!";
        return false;
    }
    if(!this->tagList) {
        errorMsg = "tag list Not Linked!";
        return false;
    }
    this->commandList = command.split("\n");
    for(int i=0;i<commandList.length();++i) {
        if(commandList[i]=="") commandList.remove(i);
    }
    if(command.isEmpty()) {
        errorMsg = "No Command!";
        return false;
    }
    int layerCount = 0;
    unit obj;
    while(currentLine<commandList.length()) {
        ++currentLine;
    }
    return true;
}

bool ifelse::begin(QString& errorMsg) {
    int flag=0;
    while(true) {
        QString s = trim(commandList[currentLine]);
        QString reserved = getAword(s);
        Expr expr;
        if(reserved=="if") {
            expr.key = Rsv::Aif;
        }
        else if(reserved=="elif") {
            expr.key = Rsv::Aelif;
        }
        else if(reserved=="else") {
            expr.key = Rsv::Aelse;
        }
        else if(reserved=="end"){
            break;
        }
        else {
            errorMsg = "Unknown Key Words.";
            flag=1;
            break;
        }
        s = trim(s);
        QString value="";
        while(!s.isEmpty()) {
            value.push_back(getAword(s));
            s=trim(s);
            if(value.back()==':') {
                value.removeLast();
                break;
            }
            if(s.isEmpty()) {
                s=commandList[++currentLine];
                s=trim(s);
            }
        }
        expr.value = value;
        expr.line = currentLine;
        this->exprlist.push_back(expr);
        s = commandList[++currentLine];
        s = trim(s);
        value.clear();
        while(!s.isEmpty() || currentLine >= endLine) {
            value.push_back(getAword(s));
            s=trim(s);
            if(value.back()==';') {
                value.removeLast();
                break;
            }
            if(s.isEmpty()) {
                s=commandList[++currentLine];
                s=trim(s);
            }
        }
        expr.line = currentLine;
        expr.value = value;
        expr.key = Rsv::Aact;
        this->actionlist.push_back(expr);
        ++currentLine;
    }
    if(flag) return false;
    else return true;
}

booleanExpr::booleanExpr() {
    this->ok = false;
    this->result = false;
}

bool booleanExpr::compare() {

}

}
*/
