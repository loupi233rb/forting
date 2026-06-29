#include "SyntaxParser.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace Forting
{
// ======================= ParseError impl =======================
ParseError::ParseError(int l, int c, const std::string& msg)
    : std::runtime_error("Parse error at " + std::to_string(l) + ":" + std::to_string(c) + ": " + msg),
      line(l),
      col(c) {}

// ======================= Internal Implementation =======================
namespace {

// ======================= Lexer =======================
enum class TokKind {
    End,
    Newline,

    Ident,
    Number,
    String,

    If,
    Elif,
    Else,
    EndKw,
    When,

    AndAnd,
    OrOr,
    Not,
    LParen,
    RParen,
    Colon,
    Semi,
    Comma,
    Plus,

    Eq,
    Ne,
    Gt,
    Lt,
    Ge,
    Le,
    Pipe,
    Dot
};

struct Token {
    TokKind kind{};
    std::string text; // raw text / identifier / string contents
    double number = 0.0;
    int line = 1, col = 1;
};

static bool is_string_valid(const std::string& s) {
    #ifdef _WIN32
    for(char c : s) {
        if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
            return false;
    }
    #elifdef __linux__
    return s.find('/') == std::string::npos && !s.empty();
    #endif
    return true;
}

class Lexer {
public:
    explicit Lexer(std::string src) : s_(std::move(src)) {}

    Token next() {
        skipSpacesExceptNewline();

        if (pos_ >= s_.size()) return make(TokKind::End, "");

        char c = s_[pos_];

        // newline
        if (c == '\n') {
            int l = line_, cl = col_;
            advance();
            Token t;
            t.kind = TokKind::Newline;
            t.text = "\n";
            t.line = l;
            t.col = cl;
            return t;
        }

        // comments (# or //)
        if (c == '#') {
            while (pos_ < s_.size() && s_[pos_] != '\n') advance();
            return next();
        }
        if (c == '/' && peek(1) == '/') {
            while (pos_ < s_.size() && s_[pos_] != '\n') advance();
            return next();
        }

        // two-char operators
        if (c == '&' && peek(1) == '&') return advance2(TokKind::AndAnd, "&&");
        if (c == '|' && peek(1) == '|') return advance2(TokKind::OrOr, "||");
        if (c == '!' && peek(1) == '=') return advance2(TokKind::Ne, "!=");
        if (c == '>' && peek(1) == '=') return advance2(TokKind::Ge, ">=");
        if (c == '<' && peek(1) == '=') return advance2(TokKind::Le, "<=");

        // single-char
        switch (c) {
            case '!': return advance1(TokKind::Not, "!");
            case '(': return advance1(TokKind::LParen, "(");
            case ')': return advance1(TokKind::RParen, ")");
            case ':': return advance1(TokKind::Colon, ":");
            case ';': return advance1(TokKind::Semi, ";");
            case ',': return advance1(TokKind::Comma, ",");
            case '+': return advance1(TokKind::Plus, "+");
            case '=': return advance1(TokKind::Eq, "=");
            case '>': return advance1(TokKind::Gt, ">");
            case '<': return advance1(TokKind::Lt, "<");
            case '|': return advance1(TokKind::Pipe, "|");
            case '.': return advance1(TokKind::Dot, ".");
            case '"': return lexString();
            default: break;
        }

        // identifier / keyword
        if (std::isalpha((unsigned char)c) || c == '_') {
            return lexIdent();
        }

        // number
        if (std::isdigit((unsigned char)c)) {
            return lexNumberLike();
        }

        throw ParseError(line_, col_, std::string("Unexpected character: '") + c + "'");
    }

private:
    std::string s_;
    std::size_t pos_ = 0;
    int line_ = 1, col_ = 1;

    char peek(int k) const {
        std::size_t p = pos_ + (std::size_t)k;
        if (p >= s_.size()) return '\0';
        return s_[p];
    }

    void advance() {
        if (pos_ >= s_.size()) return;
        if (s_[pos_] == '\n') {
            line_++;
            col_ = 1;
        } else {
            col_++;
        }
        pos_++;
    }

    void skipSpacesExceptNewline() {
        while (pos_ < s_.size()) {
            char c = s_[pos_];
            if (c == ' ' || c == '\t' || c == '\r')
                advance();
            else
                break;
        }
    }

    Token make(TokKind k, std::string text) {
        Token t;
        t.kind = k;
        t.text = std::move(text);
        t.line = line_;
        t.col = col_;
        return t;
    }

    Token advance1(TokKind k, const char* txt) {
        Token t;
        t.kind = k;
        t.text = txt;
        t.line = line_;
        t.col = col_;
        advance();
        return t;
    }

    Token advance2(TokKind k, const char* txt) {
        Token t;
        t.kind = k;
        t.text = txt;
        t.line = line_;
        t.col = col_;
        advance();
        advance();
        return t;
    }

    Token lexIdent() {
        int l = line_, c0 = col_;
        std::size_t start = pos_;
        while (pos_ < s_.size()) {
            char c = s_[pos_];
            if (std::isalnum((unsigned char)c) || c == '_')
                advance();
            else
                break;
        }
        std::string text = s_.substr(start, pos_ - start);

        TokKind k = TokKind::Ident;
        if (text == "if")
            k = TokKind::If;
        else if (text == "elif")
            k = TokKind::Elif;
        else if (text == "else")
            k = TokKind::Else;
        else if (text == "end")
            k = TokKind::EndKw;
        else if (text == "when")
            k = TokKind::When;

        Token t;
        t.kind = k;
        t.text = text;
        t.line = l;
        t.col = c0;
        return t;
    }

    Token lexString() {
        int l = line_, c0 = col_;
        advance(); // consume opening "

        std::string out;
        while (pos_ < s_.size()) {
            char c = s_[pos_];
            if (c == '"') {
                advance();
                break;
            }
            if (c == '\n' || c == '\0') {
                throw ParseError(l, c0, "Unterminated string literal");
            }
            if (c == '\\') {
                advance();
                if (pos_ >= s_.size()) throw ParseError(l, c0, "Bad escape sequence");
                char e = s_[pos_];
                switch (e) {
                    case '\\': out.push_back('\\'); break;
                    case '"': out.push_back('"'); break;
                    case 'n': out.push_back('\n'); break;
                    case 't': out.push_back('\t'); break;
                    case 'r': out.push_back('\r'); break;
                    default: throw ParseError(line_, col_, std::string("Unsupported escape: \\") + e);
                }
                advance();
            } else {
                out.push_back(c);
                advance();
            }
        }

        Token t;
        t.kind = TokKind::String;
        t.text = std::move(out);
        t.line = l;
        t.col = c0;
        return t;
    }

    Token lexNumberLike() {
        int l = line_, c0 = col_;
        std::size_t start = pos_;
        bool dot = false;
        while (pos_ < s_.size()) {
            char c = s_[pos_];
            if (std::isdigit((unsigned char)c)) {
                advance();
                continue;
            }
            if (c == '.' && !dot) {
                dot = true;
                advance();
                continue;
            }
            break;
        }

        bool mixedAlnum = false;
        if (pos_ < s_.size() && (std::isalpha((unsigned char)s_[pos_]) || s_[pos_] == '_')) {
            mixedAlnum = true;
            while (pos_ < s_.size()) {
                char c = s_[pos_];
                if (std::isalnum((unsigned char)c) || c == '_')
                    advance();
                else
                    break;
            }
        }

        std::string text = s_.substr(start, pos_ - start);

        Token t;
        t.kind = mixedAlnum ? TokKind::Ident : TokKind::Number;
        t.text = text;
        if (!mixedAlnum) t.number = std::stod(text);
        t.line = l;
        t.col = c0;
        return t;
    }
};

// ======================= Utilities =======================
static inline std::string toLower(std::string x) {
    for (char& c : x) c = (char)std::tolower((unsigned char)c);
    return x;
}

static std::int64_t parseSizeToBytes(double value, const std::string& unitRaw, int line, int col) {
    std::string u = toLower(unitRaw);
    long double mul = 1.0;
    if (u.empty() || u == "b")
        mul = 1.0;
    else if (u == "kb")
        mul = 1024.0;
    else if (u == "mb")
        mul = 1024.0L * 1024.0L;
    else if (u == "gb")
        mul = 1024.0L * 1024.0L * 1024.0L;
    else
        throw ParseError(line, col, "Unknown size unit: " + unitRaw);

    long double bytes = (long double)value * mul;
    if (bytes < 0) bytes = 0;
    return (std::int64_t)llround((double)bytes);
}

static bool parseSizeLiteralText(const std::string& text, double& value, std::string& unit) {
    std::size_t end = 0;
    try {
        value = std::stod(text, &end);
    } catch (...) {
        return false;
    }
    if (end == 0) return false;
    unit = text.substr(end);
    return true;
}

static std::string formatSizeHuman(std::int64_t bytes) {
    auto fmt = [](long double v) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss << std::setprecision(2) << (double)v;
        std::string s = oss.str();
        while (s.size() > 1 && s.find('.') != std::string::npos && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
        return s;
    };

    const long double KB = 1024.0;
    const long double MB = 1024.0L * 1024.0L;
    const long double GB = 1024.0L * 1024.0L * 1024.0L;

    long double v = (long double)bytes;
    if (v >= GB) return fmt(v / GB) + "GB";
    if (v >= MB) return fmt(v / MB) + "MB";
    if (v >= KB) return fmt(v / KB) + "KB";
    return std::to_string(bytes) + "B";
}

static int getTmYear(const std::tm& t) { return t.tm_year + 1900; }
static int getTmMonth(const std::tm& t) { return t.tm_mon + 1; }
static int getTmDay(const std::tm& t) { return t.tm_mday; }
static int getTmHour(const std::tm& t) { return t.tm_hour; }
static int getTmMinute(const std::tm& t) { return t.tm_min; }
static int getTmSecond(const std::tm& t) { return t.tm_sec; }

// ======================= AST =======================
enum class AttrBase { Name, Suffix, Size, Date, Time };
enum class AttrField { None, Year, Month, Day, Hour, Minute, Second };

struct AttrRef {
    AttrBase base{};
    AttrField field = AttrField::None;
};

enum class CmpOp { Eq, Ne, Gt, Lt, Ge, Le };

enum class FilterMode { None, White, Black };

struct Value {
    enum class Kind { Str, Number, SizeBytes, DateYMD, TimeHMS, IdentBare } kind{};
    std::string s;
    double num = 0.0;
    std::int64_t bytes = 0;
    int a = 0, b = 0, c = 0;
};

struct BoolExpr {
    virtual ~BoolExpr() = default;
    virtual bool eval(const FileEntry& f) const = 0;
};

struct StrExpr {
    virtual ~StrExpr() = default;
    virtual std::string eval(const FileEntry& f) const = 0;
};

struct StrLiteral : StrExpr {
    std::string v;
    explicit StrLiteral(std::string x) : v(std::move(x)) {}
    std::string eval(const FileEntry&) const override { return v; }
};

struct StrConcat : StrExpr {
    std::unique_ptr<StrExpr> a, b;
    StrConcat(std::unique_ptr<StrExpr> x, std::unique_ptr<StrExpr> y) : a(std::move(x)), b(std::move(y)) {}
    std::string eval(const FileEntry& f) const override { return a->eval(f) + b->eval(f); }
};

struct AttrToStr : StrExpr {
    AttrRef r;
    explicit AttrToStr(AttrRef rr) : r(rr) {}
    std::string eval(const FileEntry& f) const override {
        switch (r.base) {
            case AttrBase::Name: return f.name;
            case AttrBase::Suffix: return f.suffix;
            case AttrBase::Size: return formatSizeHuman(f.size);
            case AttrBase::Date: {
                int y = getTmYear(f.lw_time), m = getTmMonth(f.lw_time), d = getTmDay(f.lw_time);
                if (r.field == AttrField::Year) return std::to_string(y);
                if (r.field == AttrField::Month) return std::to_string(m);
                if (r.field == AttrField::Day) return std::to_string(d);
                return std::to_string(y) + "-" + std::to_string(m) + "-" + std::to_string(d);
            }
            case AttrBase::Time: {
                int h = getTmHour(f.lw_time), mi = getTmMinute(f.lw_time), se = getTmSecond(f.lw_time);
                if (r.field == AttrField::Hour) return std::to_string(h);
                if (r.field == AttrField::Minute) return std::to_string(mi);
                if (r.field == AttrField::Second) return std::to_string(se);
                return std::to_string(h) + "-" + std::to_string(mi) + "-" + std::to_string(se);
            }
        }
        return {};
    }
};

struct BoolNot : BoolExpr {
    std::unique_ptr<BoolExpr> x;
    explicit BoolNot(std::unique_ptr<BoolExpr> e) : x(std::move(e)) {}
    bool eval(const FileEntry& f) const override { return !x->eval(f); }
};

struct BoolAnd : BoolExpr {
    std::unique_ptr<BoolExpr> a, b;
    BoolAnd(std::unique_ptr<BoolExpr> x, std::unique_ptr<BoolExpr> y) : a(std::move(x)), b(std::move(y)) {}
    bool eval(const FileEntry& f) const override { return a->eval(f) && b->eval(f); }
};

struct BoolOr : BoolExpr {
    std::unique_ptr<BoolExpr> a, b;
    BoolOr(std::unique_ptr<BoolExpr> x, std::unique_ptr<BoolExpr> y) : a(std::move(x)), b(std::move(y)) {}
    bool eval(const FileEntry& f) const override { return a->eval(f) || b->eval(f); }
};

struct Compare : BoolExpr {
    AttrRef attr;
    CmpOp op;
    std::vector<Value> values;

    Compare(AttrRef a, CmpOp o, std::vector<Value> v) : attr(a), op(o), values(std::move(v)) {}

    static std::int64_t fileAttrAsBytes(const FileEntry& f, const AttrRef& ar) {
        if (ar.base != AttrBase::Size) return 0;
        return f.size;
    }

    static std::string fileAttrAsString(const FileEntry& f, const AttrRef& ar) {
        if (ar.base == AttrBase::Name) return f.name;
        if (ar.base == AttrBase::Suffix) return string(f.suffix);
        return {};
    }

    static int fileAttrAsInt(const FileEntry& f, const AttrRef& ar) {
        if (ar.base == AttrBase::Date) {
            switch (ar.field) {
                case AttrField::Year: return getTmYear(f.lw_time);
                case AttrField::Month: return getTmMonth(f.lw_time);
                case AttrField::Day: return getTmDay(f.lw_time);
                default: break;
            }
            return getTmYear(f.lw_time) * 10000 + getTmMonth(f.lw_time) * 100 + getTmDay(f.lw_time);
        }
        if (ar.base == AttrBase::Time) {
            switch (ar.field) {
                case AttrField::Hour: return getTmHour(f.lw_time);
                case AttrField::Minute: return getTmMinute(f.lw_time);
                case AttrField::Second: return getTmSecond(f.lw_time);
                default: break;
            }
            return getTmHour(f.lw_time) * 10000 + getTmMinute(f.lw_time) * 100 + getTmSecond(f.lw_time);
        }
        return 0;
    }

    bool cmpNum(long double lhs, long double rhs) const {
        switch (op) {
            case CmpOp::Eq: return lhs == rhs;
            case CmpOp::Ne: return lhs != rhs;
            case CmpOp::Gt: return lhs > rhs;
            case CmpOp::Lt: return lhs < rhs;
            case CmpOp::Ge: return lhs >= rhs;
            case CmpOp::Le: return lhs <= rhs;
        }
        return false;
    }

    bool cmpStr(const std::string& lhs, const std::string& rhs) const {
        switch (op) {
            case CmpOp::Eq: return lhs == rhs;
            case CmpOp::Ne: return lhs != rhs;
            case CmpOp::Gt: return lhs > rhs;
            case CmpOp::Lt: return lhs < rhs;
            case CmpOp::Ge: return lhs >= rhs;
            case CmpOp::Le: return lhs <= rhs;
        }
        return false;
    }

    bool evalOne(const FileEntry& f, const Value& v) const {
        if (attr.base == AttrBase::Name || attr.base == AttrBase::Suffix) {
            if (v.kind != Value::Kind::Str && v.kind != Value::Kind::IdentBare) return false;
            std::string rhs = v.s;
            return cmpStr(fileAttrAsString(f, attr), rhs);
        }
        if (attr.base == AttrBase::Size) {
            if (v.kind != Value::Kind::SizeBytes && v.kind != Value::Kind::Number) return false;
            long double rhs = (v.kind == Value::Kind::SizeBytes) ? (long double)v.bytes : (long double)v.num;
            return cmpNum((long double)fileAttrAsBytes(f, attr), rhs);
        }
        if (attr.base == AttrBase::Date) {
            long double lhs = (long double)fileAttrAsInt(f, attr);
            long double rhs = 0;
            if (v.kind == Value::Kind::DateYMD)
                rhs = (long double)(v.a * 10000 + v.b * 100 + v.c);
            else if (v.kind == Value::Kind::Number)
                rhs = (long double)v.num;
            else
                return false;
            return cmpNum(lhs, rhs);
        }
        if (attr.base == AttrBase::Time) {
            long double lhs = (long double)fileAttrAsInt(f, attr);
            long double rhs = 0;
            if (v.kind == Value::Kind::TimeHMS)
                rhs = (long double)(v.a * 10000 + v.b * 100 + v.c);
            else if (v.kind == Value::Kind::Number)
                rhs = (long double)v.num;
            else
                return false;
            return cmpNum(lhs, rhs);
        }
        return false;
    }

    bool eval(const FileEntry& f) const override {
        if (op == CmpOp::Eq && values.size() >= 1) {
            for (auto& v : values)
                if (evalOne(f, v)) return true;
            return false;
        }
        if (values.size() != 1) return false;
        return evalOne(f, values[0]);
    }
};

// ======================= Actions =======================
enum class ActKind { Tag, Rename, Delete, Nothing };

struct ActStmt {
    ActKind kind;
    std::unique_ptr<StrExpr> expr; // null for delete
};

static void applyActs(const std::vector<ActStmt>& acts, const FileEntry& f, action& out) {
    for (auto& a : acts) {
        string s;
        switch (a.kind) {
            case ActKind::Tag:
                s = a.expr->eval(f);
                out.paths /= s;
                break;
            case ActKind::Rename:
                s = a.expr->eval(f);
                out.renameValue = s;
                break;
            case ActKind::Delete:
                out.deleteFlag = true;
                break;
            case ActKind::Nothing:
                break;
        }
    }
}

// ======================= Units =======================
struct Unit {
    virtual ~Unit() = default;
    virtual void evalOneFile(const FileEntry& f, action &ac) const = 0;
};

struct TagOnlyUnit : Unit {
    std::vector<ActStmt> acts;
    explicit TagOnlyUnit(std::vector<ActStmt> a) : acts(std::move(a)) {}
    void evalOneFile(const FileEntry& f, action &ac) const override {
        applyActs(acts, f, ac);
    }
};

struct IfUnit : Unit {
    struct Branch {
        std::unique_ptr<BoolExpr> cond; // null for else
        std::vector<ActStmt> acts;
    };
    std::vector<Branch> branches;

    void evalOneFile(const FileEntry& f, action &ac) const override {
        for (auto& br : branches) {
            if (!br.cond || br.cond->eval(f)) {
                applyActs(br.acts, f, ac);
                break;
            }
        }
    }
};

struct WhenUnit : Unit {
    AttrRef attr;
    struct WBranch {
        CmpOp op;
        std::vector<Value> values;
        std::vector<ActStmt> acts;
    };
    std::vector<WBranch> branches;

    void evalOneFile(const FileEntry& f, action &ac) const override {
        for (auto& br : branches) {
            Compare cmp(attr, br.op, br.values);
            if (cmp.eval(f)) {
                applyActs(br.acts, f, ac);
                break;
            }
        }
    }
};

// ======================= Parser =======================
class Parser {
public:
    explicit Parser(std::string src) : lex_(std::move(src)) { cur_ = lex_.next(); }

    std::vector<std::unique_ptr<Unit>> parseProgram() {
        std::vector<std::unique_ptr<Unit>> units;
        consumeNewlines();

        // Parse optional white/black filter directive at the top
        parseFilterDirective();

        while (cur_.kind != TokKind::End) {
            units.push_back(parseUnit());
            consumeNewlines();
        }
        return units;
    }

    FilterMode filterMode() const { return filterMode_; }
    const std::vector<std::string>& filterFolders() const { return filterFolders_; }

private:
    Lexer lex_;
    Token cur_;
    FilterMode filterMode_ = FilterMode::None;
    std::vector<std::string> filterFolders_;

    [[noreturn]] void errorHere(const std::string& msg) { throw ParseError(cur_.line, cur_.col, msg); }

    void advance() { cur_ = lex_.next(); }

    bool accept(TokKind k) {
        if (cur_.kind == k) {
            advance();
            return true;
        }
        return false;
    }

    void expect(TokKind k, const std::string& what) {
        if (cur_.kind != k) errorHere("Expected " + what + ", got '" + cur_.text + "'");
        advance();
    }

    void consumeNewlines() {
        while (cur_.kind == TokKind::Newline) advance();
    }

    // -------- top-level filter directive --------
    void parseFilterDirective() {
        if (cur_.kind == TokKind::Ident && (cur_.text == "white" || cur_.text == "black")) {
            filterMode_ = (cur_.text == "white") ? FilterMode::White : FilterMode::Black;
            advance();
            expect(TokKind::LParen, "'(' after " + (filterMode_ == FilterMode::White ? std::string("white") : std::string("black")));
            // Parse comma-separated string list
            do {
                if (cur_.kind != TokKind::String)
                    errorHere("Expected string literal in white/black list");
                filterFolders_.push_back(cur_.text);
                advance();
            } while (accept(TokKind::Comma));
            expect(TokKind::RParen, "')' after white/black list");
            expect(TokKind::Semi, "';' after white/black directive");
            consumeNewlines();
        }
    }

    // -------- unit --------
    std::unique_ptr<Unit> parseUnit() {
        if (cur_.kind == TokKind::If) return parseIfUnit();
        if (cur_.kind == TokKind::When) return parseWhenUnit();
        return parseActUnit();
    }

    std::unique_ptr<Unit> parseActUnit() {
        auto acts = parseActList();
        expect(TokKind::Semi, "';'");
        return std::make_unique<TagOnlyUnit>(std::move(acts));
    }

    std::unique_ptr<Unit> parseIfUnit() {
        expect(TokKind::If, "'if'");
        auto cond = parseBoolExpr();
        expect(TokKind::Colon, "':' after if condition");
        consumeNewlines();
        auto ifActs = parseActList();
        expect(TokKind::Semi, "';'");
        consumeNewlines();

        auto unit = std::make_unique<IfUnit>();
        unit->branches.push_back(IfUnit::Branch{std::move(cond), std::move(ifActs)});

        while (cur_.kind == TokKind::Elif) {
            advance();
            auto c = parseBoolExpr();
            expect(TokKind::Colon, "':' after elif condition");
            consumeNewlines();
            auto a = parseActList();
            expect(TokKind::Semi, "';'");
            consumeNewlines();
            unit->branches.push_back(IfUnit::Branch{std::move(c), std::move(a)});
        }

        if (cur_.kind == TokKind::Else) {
            advance();
            expect(TokKind::Colon, "':' after else");
            consumeNewlines();
            auto a = parseActList();
            expect(TokKind::Semi, "';'");
            consumeNewlines();
            unit->branches.push_back(IfUnit::Branch{nullptr, std::move(a)});
        }

        expect(TokKind::EndKw, "'end'");
        return unit;
    }

    std::unique_ptr<Unit> parseWhenUnit() {
        expect(TokKind::When, "'when'");
        AttrRef attr = parseAttrRef();
        expect(TokKind::Colon, "':' after when attribute");
        consumeNewlines();

        auto unit = std::make_unique<WhenUnit>();
        unit->attr = attr;

        while (cur_.kind != TokKind::EndKw) {
            if (cur_.kind == TokKind::Newline) {
                advance();
                continue;
            }

            CmpOp op = parseCmpOp();

            std::vector<Value> vals;
            vals.push_back(parseValueLiteralForAttr(attr, /*allowBareIdent=*/true));
            if (op == CmpOp::Eq) {
                while (accept(TokKind::Pipe)) {
                    vals.push_back(parseValueLiteralForAttr(attr, /*allowBareIdent=*/true));
                }
            } else {
                if (cur_.kind == TokKind::Pipe) errorHere("Only '=' supports '|' multi-values");
            }

            expect(TokKind::Colon, "':' after when-branch condition");
            consumeNewlines();

            auto acts = parseActList();
            expect(TokKind::Semi, "';'");
            consumeNewlines();

            unit->branches.push_back(WhenUnit::WBranch{op, std::move(vals), std::move(acts)});
        }

        expect(TokKind::EndKw, "'end'");
        return unit;
    }

    // -------- actions --------
    std::vector<ActStmt> parseActList() {
        std::vector<ActStmt> acts;
        consumeNewlines();
        acts.push_back(parseOneAct());
        while (accept(TokKind::Comma)) {
            consumeNewlines();
            acts.push_back(parseOneAct());
        }
        return acts;
    }

    ActStmt parseOneAct() {
        if (cur_.kind == TokKind::Ident) {
            std::string kw = cur_.text;
            int l = cur_.line, c = cur_.col;
            advance();

            if (kw == "tag") {
                expect(TokKind::LParen, "'(' after tag");
                auto e = parseStrExpr();
                expect(TokKind::RParen, "')' after tag(...)");
                return ActStmt{ActKind::Tag, std::move(e)};
            }
            if (kw == "rename") {
                expect(TokKind::LParen, "'(' after rename");
                auto e = parseStrExpr();
                expect(TokKind::RParen, "')' after rename(...)");
                return ActStmt{ActKind::Rename, std::move(e)};
            }
            if (kw == "delete") {
                return ActStmt{ActKind::Delete, nullptr};
            }
            if (kw == "nothing") {
                return ActStmt{ActKind::Nothing, nullptr};
            }

            throw ParseError(l, c, "Unknown action keyword: " + kw);
        }
        errorHere("Expected action: tag(...), rename(...), delete, or nothing");
        return {};
    }

    // -------- bool expr --------
    std::unique_ptr<BoolExpr> parseBoolExpr() { return parseOr(); }

    std::unique_ptr<BoolExpr> parseOr() {
        auto left = parseAnd();
        while (accept(TokKind::OrOr)) {
            auto right = parseAnd();
            left = std::make_unique<BoolOr>(std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<BoolExpr> parseAnd() {
        auto left = parseUnary();
        while (accept(TokKind::AndAnd)) {
            auto right = parseUnary();
            left = std::make_unique<BoolAnd>(std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<BoolExpr> parseUnary() {
        if (accept(TokKind::Not)) {
            auto x = parseUnary();
            return std::make_unique<BoolNot>(std::move(x));
        }
        if (accept(TokKind::LParen)) {
            auto e = parseBoolExpr();
            expect(TokKind::RParen, "')'");
            return e;
        }

        AttrRef a = parseAttrRef();
        CmpOp op = parseCmpOp();

        std::vector<Value> vals;
        vals.push_back(parseValueLiteralForAttr(a, /*allowBareIdent=*/true));
        if (op == CmpOp::Eq) {
            while (accept(TokKind::Pipe)) {
                vals.push_back(parseValueLiteralForAttr(a, /*allowBareIdent=*/true));
            }
        } else {
            if (cur_.kind == TokKind::Pipe) errorHere("Only '=' supports '|' multi-values");
        }
        return std::make_unique<Compare>(a, op, std::move(vals));
    }

    // -------- cmp op --------
    CmpOp parseCmpOp() {
        switch (cur_.kind) {
            case TokKind::Eq: advance(); return CmpOp::Eq;
            case TokKind::Ne: advance(); return CmpOp::Ne;
            case TokKind::Gt: advance(); return CmpOp::Gt;
            case TokKind::Lt: advance(); return CmpOp::Lt;
            case TokKind::Ge: advance(); return CmpOp::Ge;
            case TokKind::Le: advance(); return CmpOp::Le;
            default: errorHere("Expected compare operator (=,!=,>,<,>=,<=)");
        }
        return CmpOp::Eq;
    }

    // -------- attribute --------
    AttrRef parseAttrRef() {
        if (cur_.kind != TokKind::Ident) errorHere("Expected attribute identifier");
        std::string base = cur_.text;
        int l = cur_.line, c = cur_.col;
        advance();

        AttrRef ar;
        if (base == "name")
            ar.base = AttrBase::Name;
        else if (base == "suffix")
            ar.base = AttrBase::Suffix;
        else if (base == "size")
            ar.base = AttrBase::Size;
        else if (base == "date")
            ar.base = AttrBase::Date;
        else if (base == "time")
            ar.base = AttrBase::Time;
        else
            throw ParseError(l, c, "Unknown attribute: " + base);

        if (accept(TokKind::Dot)) {
            if (cur_.kind != TokKind::Ident) errorHere("Expected field after '.'");
            std::string field = cur_.text;
            int l2 = cur_.line, c2 = cur_.col;
            advance();

            if (ar.base == AttrBase::Date) {
                if (field == "year")
                    ar.field = AttrField::Year;
                else if (field == "month")
                    ar.field = AttrField::Month;
                else if (field == "day")
                    ar.field = AttrField::Day;
                else
                    throw ParseError(l2, c2, "Unknown date field: " + field);
            } else if (ar.base == AttrBase::Time) {
                if (field == "hour")
                    ar.field = AttrField::Hour;
                else if (field == "minute")
                    ar.field = AttrField::Minute;
                else if (field == "second")
                    ar.field = AttrField::Second;
                else
                    throw ParseError(l2, c2, "Unknown time field: " + field);
            } else {
                throw ParseError(l2, c2, "Only 'date' or 'time' can have subfields");
            }
        }

        return ar;
    }

    // -------- value literal (same behavior as original single-file version) --------
    Value parseValueLiteralForAttr(const AttrRef& attr, bool allowBareIdent) {
        Value v;

        if (cur_.kind == TokKind::String) {
            v.kind = Value::Kind::Str;
            v.s = cur_.text;
            advance();
            return v;
        }

        if (attr.base == AttrBase::Suffix && (cur_.kind == TokKind::Number || cur_.kind == TokKind::Ident)) {
            v.kind = Value::Kind::Str;
            v.s = cur_.text;
            advance();
            return v;
        }

        if (attr.base == AttrBase::Size && cur_.kind == TokKind::Ident) {
            double num = 0.0;
            std::string unit;
            if (!parseSizeLiteralText(cur_.text, num, unit)) {
                errorHere("Expected size literal");
            }
            int l = cur_.line, c = cur_.col;
            advance();
            v.kind = Value::Kind::SizeBytes;
            v.bytes = parseSizeToBytes(num, unit.empty() ? "b" : unit, l, c);
            return v;
        }

        if (cur_.kind == TokKind::Number) {
            // Capture current token position for better error messages
            int l = cur_.line, c = cur_.col;

            double num = cur_.number;
            advance();

            // Special handling: size supports optional unit after the number (no-space or spaced both work)
            // Examples:
            //   size > 100kb
            //   size > 100 kb
            if (attr.base == AttrBase::Size) {
                std::string unit = "b"; // default
                if (cur_.kind == TokKind::Ident) {
                    double parsedValue = 0.0;
                    std::string parsedUnit;
                    if (parseSizeLiteralText(cur_.text, parsedValue, parsedUnit)) {
                        num = parsedValue;
                        unit = parsedUnit.empty() ? "b" : parsedUnit;
                        advance();
                    } else {
                        std::string u = toLower(cur_.text);
                        if (u == "b" || u == "kb" || u == "mb" || u == "gb") {
                            unit = cur_.text; // keep raw (parseSizeToBytes lowercases internally)
                            advance();
                        }
                    }
                }

                v.kind = Value::Kind::SizeBytes;
                v.bytes = parseSizeToBytes(num, unit, l, c);
                return v;
            }

            // Non-size attributes: keep as number
            v.kind = Value::Kind::Number;
            v.num = num;
            return v;
        }

        if (cur_.kind == TokKind::Ident && allowBareIdent) {
            // suffix=pdf ; treat bare ident as string
            v.kind = Value::Kind::IdentBare;
            v.s = cur_.text;
            advance();
            return v;
        }

        errorHere("Expected value literal");
        return v;
    }

    // -------- string expr --------
    std::unique_ptr<StrExpr> parseStrExpr() {
        auto left = parseStrAtom();
        while (accept(TokKind::Plus)) {
            auto right = parseStrAtom();
            left = std::make_unique<StrConcat>(std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<StrExpr> parseStrAtom() {
        if (cur_.kind == TokKind::String) {
            std::string v = cur_.text;
            if(!is_string_valid(v)) {
                errorHere("String contains invalid characters: " + v);
            }
            advance();
            return std::make_unique<StrLiteral>(std::move(v));
        }
        if (cur_.kind == TokKind::Ident) {
            AttrRef ar = parseAttrRef();
            return std::make_unique<AttrToStr>(ar);
        }
        errorHere("Expected string atom (\"literal\" or attribute reference)");
        return {};
    }
};

} // namespace

// ======================= SyntaxParser::Impl =======================
struct SyntaxParser::Impl {
    std::vector<std::unique_ptr<Unit>> units;
    FilterMode filterMode = FilterMode::None;
    std::vector<std::string> filterFolders;

    void loadFromString(const std::string& code) {
        Parser p(code);
        units = p.parseProgram();
        filterMode = p.filterMode();
        filterFolders = p.filterFolders();
    }

    void loadFromFile(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) throw std::runtime_error("Cannot open code file: " + path);
        std::ostringstream ss;
        ss << in.rdbuf();
        loadFromString(ss.str());
    }

    std::vector<action> run(const File& file) const {
        const auto& files = file.getFileList();
        const auto& srcPaths = file.getSrcPaths();
        const auto& basePath = file.current_path;
        std::vector<action> out;
        out.reserve(files.size());
        for(int i = 0; i < files.size(); ++i) {
            const auto& f = files[i];
            const auto& srcPath = fs::relative(srcPaths[i], basePath);
            action ac;
            for(auto& u : units) {
                u->evalOneFile(f, ac);
            }

            // Apply white/black filter — match against directory components only
            if (filterMode != FilterMode::None && !ac.paths.empty()) {
                bool matches = false;
                fs::path relDir = srcPath.parent_path(); // directory part only, no filename
                for (auto& folder : filterFolders) {
                    for (auto& component : relDir) {
                        if (component.string().find(folder) != std::string::npos) {
                            matches = true;
                            break;
                        }
                    }
                    if (matches) break;
                }
                bool keep = (filterMode == FilterMode::White) ? matches : !matches;
                if (!keep) {
                    ac = action{}; // Reset — no tag, rename, or delete
                }
            }

            out.push_back(std::move(ac));
        }
        return out;
    }
};

// ======================= SyntaxParser public API =======================
SyntaxParser::SyntaxParser() : impl_(std::make_unique<Impl>()) {}
SyntaxParser::~SyntaxParser() = default;

SyntaxParser::SyntaxParser(SyntaxParser&&) noexcept = default;
SyntaxParser& SyntaxParser::operator=(SyntaxParser&&) noexcept = default;

void SyntaxParser::loadFromFile(const std::string& path) { impl_->loadFromFile(path); }
void SyntaxParser::loadFromString(const std::string& code) { impl_->loadFromString(code); }

std::vector<action> SyntaxParser::run(const File& file) const {
    return impl_->run(file);
}

std::size_t SyntaxParser::unitCount() const { return impl_->units.size(); }
}