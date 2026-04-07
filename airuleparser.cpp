#include "airuleparser.h"
#include <QStringList>

namespace Forting
{

// ================= Lexer =================
RuleParser::Lexer::Lexer(const QString& src) : m_src(src) {}

QChar RuleParser::Lexer::curr() const {
    if (m_pos >= m_src.size()) return QChar();
    return m_src[m_pos];
}
QChar RuleParser::Lexer::look(int k) const {
    int p = m_pos + k;
    if (p >= m_src.size()) return QChar();
    return m_src[p];
}
bool RuleParser::Lexer::atEnd() const { return m_pos >= m_src.size(); }

void RuleParser::Lexer::advance() {
    if (atEnd()) return;
    if (m_src[m_pos] == '\n') {
        ++m_line;
        m_col = 1;
    } else {
        ++m_col;
    }
    ++m_pos;
}

RuleParser::Token RuleParser::Lexer::make(TokenType t, const QString& text, int l, int c) {
    Token tk;
    tk.type = t;
    tk.text = text;
    tk.line = l;
    tk.col = c;
    return tk;
}

void RuleParser::Lexer::skipSpacesAndComments() {
    while (!atEnd()) {
        QChar c = curr();
        if (c.isSpace()) {
            advance();
            continue;
        }
        // # ... 注释
        if (c == '#') {
            while (!atEnd() && curr() != '\n') advance();
            continue;
        }
        break;
    }
}

RuleParser::Token RuleParser::Lexer::readIdentifierOrKeyword() {
    int l = m_line, c = m_col;
    QString s;
    while (!atEnd()) {
        QChar ch = curr();
        if (ch.isLetterOrNumber() || ch == '_') {
            s.push_back(ch);
            advance();
        } else break;
    }

    const QString low = s.toLower();
    auto kw = [&](const QString& k, TokenType t)->Token {
        if (low == k) return make(t, s, l, c);
        return Token{};
    };

    Token tk;
    tk = kw("if", TokenType::KwIf); if (tk.type != TokenType::End) return tk;
    tk = kw("elif", TokenType::KwElif); if (tk.type != TokenType::End) return tk;
    tk = kw("else", TokenType::KwElse); if (tk.type != TokenType::End) return tk;
    tk = kw("end", TokenType::KwEnd); if (tk.type != TokenType::End) return tk;
    tk = kw("when", TokenType::KwWhen); if (tk.type != TokenType::End) return tk;

    tk = kw("tag", TokenType::KwTag); if (tk.type != TokenType::End) return tk;
    tk = kw("rename", TokenType::KwRename); if (tk.type != TokenType::End) return tk;
    tk = kw("delete", TokenType::KwDelete); if (tk.type != TokenType::End) return tk;

    tk = kw("name", TokenType::KwName); if (tk.type != TokenType::End) return tk;
    tk = kw("suffix", TokenType::KwSuffix); if (tk.type != TokenType::End) return tk;
    tk = kw("size", TokenType::KwSize); if (tk.type != TokenType::End) return tk;

    tk = kw("created", TokenType::KwCreated); if (tk.type != TokenType::End) return tk;
    tk = kw("modified", TokenType::KwModified); if (tk.type != TokenType::End) return tk;

    tk = kw("year", TokenType::KwYear); if (tk.type != TokenType::End) return tk;
    tk = kw("month", TokenType::KwMonth); if (tk.type != TokenType::End) return tk;
    tk = kw("day", TokenType::KwDay); if (tk.type != TokenType::End) return tk;
    tk = kw("yearmonth", TokenType::KwYearMonth); if (tk.type != TokenType::End) return tk;
    tk = kw("yearmonthday", TokenType::KwYearMonthDay); if (tk.type != TokenType::End) return tk;

    return make(TokenType::Identifier, s, l, c);
}

RuleParser::Token RuleParser::Lexer::readNumberOrDateLike() {
    // 支持: 123  10mb  2026-3-10  2026-03
    int l = m_line, c = m_col;
    QString s;
    while (!atEnd()) {
        QChar ch = curr();
        if (ch.isDigit() || ch.isLetter() || ch == '-' || ch == '.') {
            s.push_back(ch);
            advance();
        } else break;
    }
    return make(TokenType::Number, s, l, c);
}

RuleParser::Token RuleParser::Lexer::readString() {
    int l = m_line, c = m_col;
    QString s;
    advance(); // skip "

    while (!atEnd()) {
        QChar ch = curr();
        if (ch == '"') {
            advance();
            return make(TokenType::String, s, l, c);
        }
        if (ch == '\\') {
            advance();
            if (atEnd()) break;
            QChar e = curr();
            switch (e.unicode()) {
            case 'n': s.push_back('\n'); break;
            case 't': s.push_back('\t'); break;
            case '"': s.push_back('"'); break;
            case '\\': s.push_back('\\'); break;
            default: s.push_back(e); break;
            }
            advance();
            continue;
        }
        s.push_back(ch);
        advance();
    }

    return make(TokenType::End, "", l, c); // 未闭合字符串
}

RuleParser::Token RuleParser::Lexer::peek() {
    if (!m_hasPeek) {
        m_peek = next();
        m_hasPeek = true;
    }
    return m_peek;
}
RuleParser::Token RuleParser::Lexer::next() {
    if (m_hasPeek) {
        m_hasPeek = false;
        return m_peek;
    }

    skipSpacesAndComments();
    if (atEnd()) return make(TokenType::End, "", m_line, m_col);

    int l = m_line, c = m_col;
    QChar ch = curr();

    if (ch.isLetter() || ch == '_') return readIdentifierOrKeyword();
    if (ch.isDigit()) return readNumberOrDateLike();
    if (ch == '"') return readString();

    // 2-char ops
    if (ch == '&' && look() == '&') { advance(); advance(); return make(TokenType::AndAnd, "&&", l, c); }
    if (ch == '|' && look() == '|') { advance(); advance(); return make(TokenType::OrOr, "||", l, c); }
    if (ch == '!' && look() == '=') { advance(); advance(); return make(TokenType::Ne, "!=", l, c); }
    if (ch == '>' && look() == '=') { advance(); advance(); return make(TokenType::Ge, ">=", l, c); }
    if (ch == '<' && look() == '=') { advance(); advance(); return make(TokenType::Le, "<=", l, c); }

    // 1-char
    advance();
    switch (ch.unicode()) {
    case '(': return make(TokenType::LParen, "(", l, c);
    case ')': return make(TokenType::RParen, ")", l, c);
    case ':': return make(TokenType::Colon, ":", l, c);
    case ';': return make(TokenType::Semicolon, ";", l, c);
    case ',': return make(TokenType::Comma, ",", l, c);
    case '+': return make(TokenType::Plus, "+", l, c);
    case '|': return make(TokenType::Pipe, "|", l, c);
    case '.': return make(TokenType::Dot, ".", l, c);
    case '!': return make(TokenType::Not, "!", l, c);
    case '=': return make(TokenType::Eq, "=", l, c);
    case '>': return make(TokenType::Gt, ">", l, c);
    case '<': return make(TokenType::Lt, "<", l, c);
    default: return make(TokenType::End, "", l, c);
    }
}

// ================= Expr Eval =================
bool RuleParser::NotExpr::eval(const FileEntry& f) const { return !e->eval(f); }

bool RuleParser::BinaryExpr::eval(const FileEntry& f) const {
    if (op == Op::And) return l->eval(f) && r->eval(f);
    return l->eval(f) || r->eval(f);
}

bool RuleParser::CondExpr::eval(const FileEntry& f) const {
    return RuleParser::compareByAttr(attr, op, raw, f);
}

QString RuleParser::StringExpr::eval(const FileEntry& f) const {
    QString out;
    for (const auto& p : parts) {
        if (p.kind == StringPart::Kind::Literal) out += p.literal;
        else out += RuleParser::attrToString(f, p.attr);
    }
    return out;
}

// ================= Parser =================
RuleParser::RuleParser() {}

bool RuleParser::parse(const QString& script) {
    m_script = script;
    m_lex = std::make_unique<Lexer>(script);
    m_units.clear();
    m_parsed = false;
    m_error = {};

    if (!parseProgram()) return false;
    m_parsed = true;
    return true;
}

QVector<action> RuleParser::evaluate(const FileEntry& file) const {
    QVector<action> out;
    if (!m_parsed) return out;

    out.reserve(m_units.size());
    for (const auto& u : m_units) {
        ActionNode an = u.resolver(file);

        action a;
        a.decision = (an.doTag ? 4 : 0) | (an.doRename ? 2 : 0) | (an.doDelete ? 1 : 0);
        a.deleteFlag = an.doDelete;

        if (an.doTag) a.tagValue = an.tagExpr.eval(file);
        if (an.doRename) a.renameValue = an.renameExpr.eval(file);

        out.push_back(a);
    }
    return out;
}

bool RuleParser::parseProgram() {
    while (true) {
        Token t = peek();
        if (t.type == TokenType::End) break;
        Unit u;
        if (!parseUnit(u)) return false;
        m_units.push_back(u);
    }
    return true;
}

bool RuleParser::parseUnit(Unit& out) {
    Token t = peek();
    if (t.type == TokenType::KwIf) return parseIfUnit(out);
    if (t.type == TokenType::KwWhen) return parseWhenUnit(out);
    return parseSingleActionUnit(out);
}

// if cond : actions ; [elif ...] [else : actions ;] end
bool RuleParser::parseIfUnit(Unit& out) {
    if (!expect(TokenType::KwIf, "期望 if")) return false;

    ExprPtr cond = parseBoolExpr();
    if (!cond) return false;
    if (!expect(TokenType::Colon, "if 条件后缺少 ':'")) return false;

    ActionNode ifAct;
    if (!parseActionList(ifAct)) return false;

    struct Branch { ExprPtr c; ActionNode a; };
    QVector<Branch> branches;
    branches.push_back({cond, ifAct});

    while (accept(TokenType::KwElif)) {
        ExprPtr ec = parseBoolExpr();
        if (!ec) return false;
        if (!expect(TokenType::Colon, "elif 条件后缺少 ':'")) return false;
        ActionNode ea;
        if (!parseActionList(ea)) return false;
        branches.push_back({ec, ea});
    }

    bool hasElse = false;
    ActionNode elseAct;
    if (accept(TokenType::KwElse)) {
        hasElse = true;
        if (!expect(TokenType::Colon, "else 后缺少 ':'")) return false;
        if (!parseActionList(elseAct)) return false;
    }

    if (!expect(TokenType::KwEnd, "if 单元缺少 end")) return false;

    out.resolver = [branches, hasElse, elseAct](const FileEntry& f)->ActionNode {
        for (const auto& b : branches) {
            if (b.c->eval(f)) return b.a;
        }
        if (hasElse) return elseAct;
        return ActionNode{};
    };
    return true;
}

// when attr : cond:actions; cond:actions; ... end
bool RuleParser::parseWhenUnit(Unit& out) {
    if (!expect(TokenType::KwWhen, "期望 when")) return false;

    AttrType whenAttr;
    if (!parseWhenAttr(whenAttr)) return false;

    if (!expect(TokenType::Colon, "when 属性后缺少 ':'")) return false;

    struct WBranch { ExprPtr c; ActionNode a; };
    QVector<WBranch> branches;

    while (true) {
        Token t = peek();
        if (t.type == TokenType::KwEnd) break;

        bool ok = true;
        ExprPtr c = parseCondition(true, whenAttr, &ok);
        if (!ok || !c) return false;

        if (!expect(TokenType::Colon, "when 分支条件后缺少 ':'")) return false;

        ActionNode a;
        if (!parseActionList(a)) return false;
        branches.push_back({c, a});
    }

    if (!expect(TokenType::KwEnd, "when 单元缺少 end")) return false;

    out.resolver = [branches](const FileEntry& f)->ActionNode {
        for (const auto& b : branches) {
            if (b.c->eval(f)) return b.a;
        }
        return ActionNode{};
    };
    return true;
}

bool RuleParser::parseSingleActionUnit(Unit& out) {
    ActionNode a;
    if (!parseActionList(a)) return false;
    out.resolver = [a](const FileEntry&) -> ActionNode { return a; };
    return true;
}

bool RuleParser::parseActionList(ActionNode& out) {
    // 支持: tag(...) , rename(...) , delete ;
    bool got = false;

    while (true) {
        Token t = peek();
        if (t.type == TokenType::KwTag) {
            next();
            if (!expect(TokenType::LParen, "tag 缺少 '('")) return false;
            StringExpr se;
            if (!parseStringExpr(se)) return false;
            if (!expect(TokenType::RParen, "tag 缺少 ')'")) return false;
            out.doTag = true;
            out.tagExpr = se;
            got = true;
        } else if (t.type == TokenType::KwRename) {
            next();
            if (!expect(TokenType::LParen, "rename 缺少 '('")) return false;
            StringExpr se;
            if (!parseStringExpr(se)) return false;
            if (!expect(TokenType::RParen, "rename 缺少 ')'")) return false;
            out.doRename = true;
            out.renameExpr = se;
            got = true;
        } else if (t.type == TokenType::KwDelete) {
            next();
            out.doDelete = true;
            got = true;
        } else {
            break;
        }

        if (accept(TokenType::Comma)) continue;
        break;
    }

    if (!got) {
        setErrorHere("期望操作代码(tag/rename/delete)");
        return false;
    }

    if (!expect(TokenType::Semicolon, "操作代码末尾缺少 ';'")) return false;
    return true;
}

// ---------- bool expr ----------
RuleParser::ExprPtr RuleParser::parseBoolExpr() { return parseOrExpr(); }

RuleParser::ExprPtr RuleParser::parseOrExpr() {
    ExprPtr left = parseAndExpr();
    if (!left) return nullptr;

    while (accept(TokenType::OrOr)) {
        ExprPtr right = parseAndExpr();
        if (!right) return nullptr;
        auto b = std::make_shared<BinaryExpr>();
        b->op = BinaryExpr::Op::Or;
        b->l = left;
        b->r = right;
        left = b;
    }
    return left;
}

RuleParser::ExprPtr RuleParser::parseAndExpr() {
    ExprPtr left = parseUnaryExpr();
    if (!left) return nullptr;

    while (accept(TokenType::AndAnd)) {
        ExprPtr right = parseUnaryExpr();
        if (!right) return nullptr;
        auto b = std::make_shared<BinaryExpr>();
        b->op = BinaryExpr::Op::And;
        b->l = left;
        b->r = right;
        left = b;
    }
    return left;
}

RuleParser::ExprPtr RuleParser::parseUnaryExpr() {
    if (accept(TokenType::Not)) {
        ExprPtr e = parseUnaryExpr();
        if (!e) return nullptr;
        auto n = std::make_shared<NotExpr>();
        n->e = e;
        return n;
    }
    return parsePrimaryExpr();
}

RuleParser::ExprPtr RuleParser::parsePrimaryExpr() {
    if (accept(TokenType::LParen)) {
        ExprPtr e = parseBoolExpr();
        if (!e) return nullptr;
        if (!expect(TokenType::RParen, "缺少 ')'")) return nullptr;
        return e;
    }

    bool ok = true;
    return parseCondition(false, AttrType::Name, &ok);
}

RuleParser::ExprPtr RuleParser::parseCondition(bool inWhen, AttrType whenAttr, bool* ok) {
    *ok = false;

    AttrType attr = whenAttr;
    if (!inWhen) {
        if (!parseAttr(attr)) return nullptr;
    }

    Token op = next();
    CmpOp cmp;
    switch (op.type) {
    case TokenType::Eq: cmp = CmpOp::EQ; break;
    case TokenType::Ne: cmp = CmpOp::NE; break;
    case TokenType::Gt: cmp = CmpOp::GT; break;
    case TokenType::Lt: cmp = CmpOp::LT; break;
    case TokenType::Ge: cmp = CmpOp::GE; break;
    case TokenType::Le: cmp = CmpOp::LE; break;
    default:
        setError(op, "缺少比较符号(=,!=,>,<,>=,<=)");
        return nullptr;
    }

    // 右值: string / number / identifier，支持 | 连接
    QString rhs;
    Token v = next();
    if (v.type != TokenType::String && v.type != TokenType::Number && v.type != TokenType::Identifier) {
        setError(v, "比较右值非法");
        return nullptr;
    }
    rhs += v.text;

    while (accept(TokenType::Pipe)) {
        Token nv = next();
        if (nv.type != TokenType::String && nv.type != TokenType::Number && nv.type != TokenType::Identifier) {
            setError(nv, "'|' 后右值非法");
            return nullptr;
        }
        rhs += "|" + nv.text;
    }

    auto c = std::make_shared<CondExpr>();
    c->attr = attr;
    c->op = cmp;
    c->raw = rhs;
    *ok = true;
    return c;
}

// stringExpr := term ('+' term)*
// term := STRING | attrRef
bool RuleParser::parseStringExpr(StringExpr& out) {
    auto parseTerm = [&](StringPart& p) -> bool {
        Token t = peek();
        if (t.type == TokenType::String) {
            next();
            p.kind = StringPart::Kind::Literal;
            p.literal = t.text;
            return true;
        }
        AttrType a;
        if (!parseAttr(a)) return false;
        p.kind = StringPart::Kind::Attr;
        p.attr = a;
        return true;
    };

    StringPart first;
    if (!parseTerm(first)) {
        setErrorHere("字符串表达式需要字符串常量或属性");
        return false;
    }
    out.parts.push_back(first);

    while (accept(TokenType::Plus)) {
        StringPart n;
        if (!parseTerm(n)) {
            setErrorHere("'+' 后缺少字符串常量或属性");
            return false;
        }
        out.parts.push_back(n);
    }

    return true;
}

// 属性：
// name
// suffix
// size | size.b | size.kb | size.mb | size.gb
// created.year / ... / created.yearmonthday
// modified.year / ... / modified.yearmonthday
bool RuleParser::parseAttr(AttrType& attr) {
    Token t = next();

    if (t.type == TokenType::KwName) { attr = AttrType::Name; return true; }
    if (t.type == TokenType::KwSuffix) { attr = AttrType::Suffix; return true; }

    if (t.type == TokenType::KwSize) {
        if (accept(TokenType::Dot)) {
            Token u = next();
            QString low = u.text.toLower();
            if (low == "b")  { attr = AttrType::SizeB;  return true; }
            if (low == "kb") { attr = AttrType::SizeKB; return true; }
            if (low == "mb") { attr = AttrType::SizeMB; return true; }
            if (low == "gb") { attr = AttrType::SizeGB; return true; }
            setError(u, "size 仅支持 .b/.kb/.mb/.gb");
            return false;
        }
        attr = AttrType::SizeB; // size 默认 bytes
        return true;
    }

    auto parseDateSub = [&](bool created) -> bool {
        if (!accept(TokenType::Dot)) {
            setErrorHere("created/modified 需要 .子字段（year/month/day/yearmonth/yearmonthday）");
            return false;
        }
        Token f = next();
        if (f.type == TokenType::KwYear) { attr = created ? AttrType::CreatedYear : AttrType::ModifiedYear; return true; }
        if (f.type == TokenType::KwMonth) { attr = created ? AttrType::CreatedMonth : AttrType::ModifiedMonth; return true; }
        if (f.type == TokenType::KwDay) { attr = created ? AttrType::CreatedDay : AttrType::ModifiedDay; return true; }
        if (f.type == TokenType::KwYearMonth) { attr = created ? AttrType::CreatedYearMonth : AttrType::ModifiedYearMonth; return true; }
        if (f.type == TokenType::KwYearMonthDay) { attr = created ? AttrType::CreatedYearMonthDay : AttrType::ModifiedYearMonthDay; return true; }
        setError(f, "未知日期子字段");
        return false;
    };

    if (t.type == TokenType::KwCreated) return parseDateSub(true);
    if (t.type == TokenType::KwModified) return parseDateSub(false);

    setError(t, "未知属性，支持 name/suffix/size(.b/.kb/.mb/.gb)/created.xxx/modified.xxx");
    return false;
}

bool RuleParser::parseWhenAttr(AttrType& attr) {
    return parseAttr(attr);
}

// ---------- token helpers ----------
RuleParser::Token RuleParser::peek() { return m_lex->peek(); }
RuleParser::Token RuleParser::next() { return m_lex->next(); }

bool RuleParser::accept(TokenType t, Token* got) {
    Token p = peek();
    if (p.type == t) {
        Token n = next();
        if (got) *got = n;
        return true;
    }
    return false;
}

bool RuleParser::expect(TokenType t, const QString& msg, Token* got) {
    Token n = next();
    if (n.type != t) {
        setError(n, msg);
        return false;
    }
    if (got) *got = n;
    return true;
}

void RuleParser::setError(const Token& t, const QString& msg) {
    if (m_error.hasError) return;
    m_error.hasError = true;
    m_error.line = t.line;
    m_error.column = t.col;
    m_error.message = msg;
}

void RuleParser::setErrorHere(const QString& msg) {
    setError(peek(), msg);
}

// ================= Utils =================
static QString dateYearMonth(const QDate& d) {
    return QString("%1-%2").arg(d.year()).arg(d.month());
}
static QString dateYearMonthDay(const QDate& d) {
    return QString("%1-%2-%3").arg(d.year()).arg(d.month()).arg(d.day());
}

QString RuleParser::attrToString(const FileEntry& f, AttrType a) {
    const QDate cd = f.created.date();
    const QDate md = f.modified.date();

    switch (a) {
    case AttrType::Name: return f.name;
    case AttrType::Suffix: return f.suffix;

    case AttrType::SizeB:  return QString::number((double)f.size, 'f', 0);
    case AttrType::SizeKB: return QString::number((double)f.size / 1024.0, 'f', 2);
    case AttrType::SizeMB: return QString::number((double)f.size / 1024.0 / 1024.0, 'f', 2);
    case AttrType::SizeGB: return QString::number((double)f.size / 1024.0 / 1024.0 / 1024.0, 'f', 4);

    case AttrType::CreatedYear: return QString::number(cd.year());
    case AttrType::CreatedMonth: return QString::number(cd.month());
    case AttrType::CreatedDay: return QString::number(cd.day());
    case AttrType::CreatedYearMonth: return dateYearMonth(cd);
    case AttrType::CreatedYearMonthDay: return dateYearMonthDay(cd);

    case AttrType::ModifiedYear: return QString::number(md.year());
    case AttrType::ModifiedMonth: return QString::number(md.month());
    case AttrType::ModifiedDay: return QString::number(md.day());
    case AttrType::ModifiedYearMonth: return dateYearMonth(md);
    case AttrType::ModifiedYearMonthDay: return dateYearMonthDay(md);
    }
    return {};
}

double RuleParser::bytesToUnit(double bytes, AttrType sizeAttrUnit) {
    switch (sizeAttrUnit) {
    case AttrType::SizeB:  return bytes;
    case AttrType::SizeKB: return bytes / 1024.0;
    case AttrType::SizeMB: return bytes / (1024.0 * 1024.0);
    case AttrType::SizeGB: return bytes / (1024.0 * 1024.0 * 1024.0);
    default: return bytes;
    }
}

// 解析右值数字（可带单位）
// "10mb" -> outValueBytes=10485760, hasUnit=true
// "10"   -> outValueBytes=10,       hasUnit=false
bool RuleParser::parseNumberWithOptionalUnit(const QString& s, double* outValueBytes, bool* hasUnit) {
    if (outValueBytes) *outValueBytes = 0.0;
    if (hasUnit) *hasUnit = false;

    QRegularExpression re(R"(^\s*([0-9]+(?:\.[0-9]+)?)\s*([a-zA-Z]*)\s*$)");
    auto m = re.match(s);
    if (!m.hasMatch()) return false;

    bool okNum = false;
    double num = m.captured(1).toDouble(&okNum);
    if (!okNum) return false;

    QString unit = m.captured(2).toLower();
    double bytes = num;
    bool hu = false;

    if (!unit.isEmpty()) {
        hu = true;
        if (unit == "b") bytes = num;
        else if (unit == "kb") bytes = num * 1024.0;
        else if (unit == "mb") bytes = num * 1024.0 * 1024.0;
        else if (unit == "gb") bytes = num * 1024.0 * 1024.0 * 1024.0;
        else return false;
    }

    if (outValueBytes) *outValueBytes = bytes;
    if (hasUnit) *hasUnit = hu;
    return true;
}

double RuleParser::attrToNumber(const FileEntry& f, AttrType a, bool* okNumeric, bool* isStringLike) {
    if (okNumeric) *okNumeric = true;
    if (isStringLike) *isStringLike = false;

    auto dNum = [](const QString& s)->double {
        // YYYY-M-D => YYYYMMDD
        auto ps = s.split('-', Qt::SkipEmptyParts);
        if (ps.size() == 1) return ps[0].toDouble();
        if (ps.size() == 2) return ps[0].toDouble() * 100.0 + ps[1].toDouble();
        if (ps.size() == 3) return ps[0].toDouble() * 10000.0 + ps[1].toDouble() * 100.0 + ps[2].toDouble();
        return 0.0;
    };

    switch (a) {
    case AttrType::SizeB:
    case AttrType::SizeKB:
    case AttrType::SizeMB:
    case AttrType::SizeGB:
        return bytesToUnit((double)f.size, a);

    case AttrType::CreatedYear:
    case AttrType::CreatedMonth:
    case AttrType::CreatedDay:
    case AttrType::CreatedYearMonth:
    case AttrType::CreatedYearMonthDay:
    case AttrType::ModifiedYear:
    case AttrType::ModifiedMonth:
    case AttrType::ModifiedDay:
    case AttrType::ModifiedYearMonth:
    case AttrType::ModifiedYearMonthDay:
        return dNum(attrToString(f, a));

    case AttrType::Name:
    case AttrType::Suffix:
        if (okNumeric) *okNumeric = false;
        if (isStringLike) *isStringLike = true;
        return 0.0;
    }
    if (okNumeric) *okNumeric = false;
    return 0.0;
}

bool RuleParser::compareOne(AttrType attr, CmpOp op, const QString& rhsOne, const FileEntry& f) {
    auto cmpNum = [&](double lv, double rv)->bool {
        switch (op) {
        case CmpOp::EQ: return lv == rv;
        case CmpOp::NE: return lv != rv;
        case CmpOp::GT: return lv > rv;
        case CmpOp::LT: return lv < rv;
        case CmpOp::GE: return lv >= rv;
        case CmpOp::LE: return lv <= rv;
        }
        return false;
    };
    auto cmpStr = [&](const QString& lv, const QString& rv)->bool {
        int c = QString::compare(lv, rv, Qt::CaseInsensitive);
        switch (op) {
        case CmpOp::EQ: return c == 0;
        case CmpOp::NE: return c != 0;
        case CmpOp::GT: return c > 0;
        case CmpOp::LT: return c < 0;
        case CmpOp::GE: return c >= 0;
        case CmpOp::LE: return c <= 0;
        }
        return false;
    };

    bool okNumeric = false, isStringLike = false;
    double lv = attrToNumber(f, attr, &okNumeric, &isStringLike);

    // name/suffix 字符串比较
    if (isStringLike) {
        return cmpStr(attrToString(f, attr), rhsOne);
    }

    // 数值/日期/size比较
    if (okNumeric) {
        // size.*：右值若无单位，则按左值单位解释；有单位则按指定单位转换
        if (attr == AttrType::SizeB || attr == AttrType::SizeKB || attr == AttrType::SizeMB || attr == AttrType::SizeGB) {
            double rvBytes = 0.0;
            bool hasUnit = false;
            if (!parseNumberWithOptionalUnit(rhsOne, &rvBytes, &hasUnit)) return false;

            double rv = 0.0;
            if (hasUnit) {
                // 已是 bytes，再转成左值单位
                rv = bytesToUnit(rvBytes, attr);
            } else {
                // 无单位，直接按左值单位数值
                bool ok = false;
                rv = rhsOne.toDouble(&ok);
                if (!ok) return false;
            }
            return cmpNum(lv, rv);
        }

        // 日期 / 纯数字
        bool ok = false;
        double rv = rhsOne.toDouble(&ok);
        if (!ok) {
            // 兼容 YYYY-M-D / YYYY-M
            auto ps = rhsOne.split('-', Qt::SkipEmptyParts);
            if (ps.size() == 2) {
                bool o1=false,o2=false;
                double y=ps[0].toDouble(&o1), m=ps[1].toDouble(&o2);
                if (!o1 || !o2) return false;
                rv = y*100.0 + m;
                ok = true;
            } else if (ps.size() == 3) {
                bool o1=false,o2=false,o3=false;
                double y=ps[0].toDouble(&o1), m=ps[1].toDouble(&o2), d=ps[2].toDouble(&o3);
                if (!o1 || !o2 || !o3) return false;
                rv = y*10000.0 + m*100.0 + d;
                ok = true;
            }
        }
        if (!ok) return false;
        return cmpNum(lv, rv);
    }

    return false;
}

bool RuleParser::compareByAttr(AttrType attr, CmpOp op, const QString& rhsRaw, const FileEntry& f) {
    const QStringList items = rhsRaw.split('|', Qt::KeepEmptyParts);

    // = / != 使用“多值集合”语义
    if (op == CmpOp::EQ) {
        for (const QString& it : items) {
            if (compareOne(attr, CmpOp::EQ, it.trimmed(), f)) return true;
        }
        return false;
    }
    if (op == CmpOp::NE) {
        // != a|b|c 等价于 “都不等于”
        for (const QString& it : items) {
            if (compareOne(attr, CmpOp::EQ, it.trimmed(), f)) return false;
        }
        return true;
    }

    // 其它比较符，若写了 |，按任一满足
    for (const QString& it : items) {
        if (compareOne(attr, op, it.trimmed(), f)) return true;
    }
    return false;
}

QString readFromFile(const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString content = stream.readAll();
    file.close();
    return content;
}

}
