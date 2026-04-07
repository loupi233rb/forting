#pragma once

#include "fortstruct.h"

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QVector>
#include <QRegularExpression>
#include <QFile>
#include <functional>
#include <memory>

namespace Forting
{
// ---------- 解析错误 ----------
struct ParseError {
    int line = 1;
    int column = 1;
    QString message;
    bool hasError = false;
};

// ---------- 解析器 ----------
class RuleParser {
public:
    RuleParser();

    // 解析规则脚本（可多行）
    bool parse(const QString& script);

    // 对单个文件执行，输出与“单元数”一一对应的 action
    QVector<action> evaluate(const FileEntry& file) const;

    bool isParsed() const { return m_parsed; }
    ParseError lastError() const { return m_error; }

private:
    // ===== Lexer =====
    enum class TokenType {
        End,
        Identifier,
        Number,
        String,

        // symbols
        LParen, RParen, Colon, Semicolon, Comma,
        Plus, Pipe, Dot,
        Not, AndAnd, OrOr,

        Eq, Ne, Gt, Lt, Ge, Le,

        // keywords
        KwIf, KwElif, KwElse, KwEnd,
        KwWhen,
        KwTag, KwRename, KwDelete,
        KwName, KwSuffix, KwSize,
        KwCreated, KwModified,
        KwYear, KwMonth, KwDay, KwYearMonth, KwYearMonthDay
    };

    struct Token {
        TokenType type = TokenType::End;
        QString text;
        int line = 1;
        int col = 1;
    };

    class Lexer {
    public:
        explicit Lexer(const QString& src);
        Token next();
        Token peek();

    private:
        QChar curr() const;
        QChar look(int k = 1) const;
        bool atEnd() const;
        void advance();
        void skipSpacesAndComments();

        Token make(TokenType t, const QString& text, int l, int c);

        Token readIdentifierOrKeyword();
        Token readNumberOrDateLike();
        Token readString();

        QString m_src;
        int m_pos = 0;
        int m_line = 1;
        int m_col = 1;

        bool m_hasPeek = false;
        Token m_peek;
    };

    // ===== AST / Eval =====
    enum class AttrType {
        Name,
        Suffix,

        // size 原始字节及单位视图
        SizeB,
        SizeKB,
        SizeMB,
        SizeGB,

        CreatedYear, CreatedMonth, CreatedDay, CreatedYearMonth, CreatedYearMonthDay,
        ModifiedYear, ModifiedMonth, ModifiedDay, ModifiedYearMonth, ModifiedYearMonthDay
    };

    enum class CmpOp { EQ, NE, GT, LT, GE, LE };

    struct Expr {
        virtual ~Expr() = default;
        virtual bool eval(const FileEntry& f) const = 0;
    };
    using ExprPtr = std::shared_ptr<Expr>;

    struct NotExpr : Expr {
        ExprPtr e;
        bool eval(const FileEntry& f) const override;
    };

    struct BinaryExpr : Expr {
        enum class Op { And, Or } op;
        ExprPtr l, r;
        bool eval(const FileEntry& f) const override;
    };

    struct CondExpr : Expr {
        AttrType attr;
        CmpOp op;
        QString raw; // 右值，可含 |
        bool eval(const FileEntry& f) const override;
    };

    struct StringPart {
        enum class Kind { Literal, Attr } kind = Kind::Literal;
        QString literal;
        AttrType attr = AttrType::Name;
    };

    struct StringExpr {
        QVector<StringPart> parts;
        QString eval(const FileEntry& f) const;
    };

    struct ActionNode {
        bool doTag = false;
        bool doRename = false;
        bool doDelete = false;

        StringExpr tagExpr;
        StringExpr renameExpr;
    };

    struct Unit {
        std::function<ActionNode(const FileEntry&)> resolver;
    };

    // ===== Parser =====
    bool parseProgram();
    bool parseUnit(Unit& out);
    bool parseIfUnit(Unit& out);
    bool parseWhenUnit(Unit& out);
    bool parseSingleActionUnit(Unit& out);

    bool parseActionList(ActionNode& out);

    ExprPtr parseBoolExpr();
    ExprPtr parseOrExpr();
    ExprPtr parseAndExpr();
    ExprPtr parseUnaryExpr();
    ExprPtr parsePrimaryExpr();

    ExprPtr parseCondition(bool inWhen, AttrType whenAttr, bool* ok);

    bool parseStringExpr(StringExpr& out);

    bool parseAttr(AttrType& attr);
    bool parseWhenAttr(AttrType& attr);

    // token helpers
    Token peek();
    Token next();
    bool accept(TokenType t, Token* got = nullptr);
    bool expect(TokenType t, const QString& msg, Token* got = nullptr);
    void setError(const Token& t, const QString& msg);
    void setErrorHere(const QString& msg);

    // utils
    static QString attrToString(const FileEntry& f, AttrType a);
    static double  attrToNumber(const FileEntry& f, AttrType a, bool* okNumeric, bool* isStringLike);
    static bool compareByAttr(AttrType attr, CmpOp op, const QString& rhsRaw, const FileEntry& f);
    static bool compareOne(AttrType attr, CmpOp op, const QString& rhsOne, const FileEntry& f);

    static bool parseNumberWithOptionalUnit(const QString& s, double* outValueBytes, bool* hasUnit);
    static double bytesToUnit(double bytes, AttrType sizeAttrUnit);

private:
    QString m_script;
    std::unique_ptr<Lexer> m_lex;
    QVector<Unit> m_units;

    bool m_parsed = false;
    ParseError m_error;
};

QString readFromFile(const QString& filepath);

}
