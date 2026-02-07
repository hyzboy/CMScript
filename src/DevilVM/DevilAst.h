#pragma once

#include <hgl/devil/DevilToken.h>
#include <hgl/devil/DevilValue.h>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace hgl::devil
{
    class Module;
    class Context;
    class Func;

    struct SourceLocation
    {
        int line=0;
        int column=0;
        std::string line_text;
        std::string caret_line;
    };

    struct ExecContext
    {
        Module *module=nullptr;
        Context *context=nullptr;
        Func *func=nullptr;
        std::unordered_map<std::string,AstValue> locals;
        SourceLocation current_loc;
        std::string error;
    };

    enum class ExecFlow
    {
        Normal,
        Return,
        Goto,
        Break,
        Continue,
        Error
    };

    struct ExecResult
    {
        ExecFlow flow=ExecFlow::Normal;
        AstValue value=AstValue::MakeVoid();
        std::string label;
        std::string error;

        static ExecResult Normal();
        static ExecResult Return(AstValue v);
        static ExecResult Goto(const std::string &label);
        static ExecResult Break();
        static ExecResult Continue();
        static ExecResult Error(const std::string &message);
    };

    class Expr
    {
    public:
        virtual ~Expr()=default;
        virtual AstValue Eval(ExecContext &) const=0;
    };

    class Stmt
    {
    public:
        virtual ~Stmt()=default;
        virtual ExecResult Exec(ExecContext &) const=0;
    };

    class LiteralExpr final:public Expr
    {
        AstValue value;

    public:
        explicit LiteralExpr(AstValue v):value(std::move(v)){}
        const AstValue &GetValue() const{return value;}
        AstValue Eval(ExecContext &) const override{return value;}
    };

    class EnumValueExpr final:public Expr
    {
        std::string enum_name;
        std::string value_name;
        AstValue value;

    public:
        EnumValueExpr(std::string e,std::string v,AstValue av)
            : enum_name(std::move(e)), value_name(std::move(v)), value(std::move(av)){}
        const std::string &GetEnumName() const{return enum_name;}
        const std::string &GetValueName() const{return value_name;}
        const AstValue &GetValue() const{return value;}
        AstValue Eval(ExecContext &) const override{return value;}
    };

    class IdentifierExpr final:public Expr
    {
        std::string name;

    public:
        explicit IdentifierExpr(std::string n):name(std::move(n)){}
        const std::string &GetName() const{return name;}
        AstValue Eval(ExecContext &) const override;
    };

    class UnaryExpr final:public Expr
    {
        TokenType op;
        std::unique_ptr<Expr> expr;

    public:
        UnaryExpr(TokenType o,std::unique_ptr<Expr> e)
            : op(o), expr(std::move(e)){}
        TokenType GetOp() const{return op;}
        const Expr *GetExpr() const{return expr.get();}
        AstValue Eval(ExecContext &) const override;
    };

    class BinaryExpr final:public Expr
    {
        TokenType op;
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;

    public:
        BinaryExpr(TokenType o,std::unique_ptr<Expr> l,std::unique_ptr<Expr> r)
            : op(o), left(std::move(l)), right(std::move(r)){}
        TokenType GetOp() const{return op;}
        const Expr *GetLeft() const{return left.get();}
        const Expr *GetRight() const{return right.get();}
        AstValue Eval(ExecContext &) const override;
    };

    class AssignExpr final:public Expr
    {
        std::string name;
        std::unique_ptr<Expr> value;

    public:
        AssignExpr(std::string n,std::unique_ptr<Expr> v)
            : name(std::move(n)), value(std::move(v)){}
        const std::string &GetName() const{return name;}
        const Expr *GetValue() const{return value.get();}
        AstValue Eval(ExecContext &) const override;
    };

    class CastExpr final:public Expr
    {
        TokenType target_type;
        std::unique_ptr<Expr> value;

    public:
        CastExpr(TokenType type,std::unique_ptr<Expr> v)
            : target_type(type), value(std::move(v)){}
        TokenType GetTargetType() const{return target_type;}
        const Expr *GetValue() const{return value.get();}
        AstValue Eval(ExecContext &) const override;
    };

    class CallExpr final:public Expr
    {
        std::string name;
        std::vector<std::unique_ptr<Expr>> args;

    public:
        CallExpr(std::string n,std::vector<std::unique_ptr<Expr>> a)
            : name(std::move(n)), args(std::move(a)){}
        const std::string &GetName() const{return name;}
        const std::vector<std::unique_ptr<Expr>> &GetArgs() const{return args;}
        AstValue Eval(ExecContext &) const override;
    };

    class BlockStmt;

    class VarDeclStmt final:public Stmt
    {
        TokenType type;
        std::string name;
        std::unique_ptr<Expr> init;

    public:
        VarDeclStmt(TokenType t,std::string n,std::unique_ptr<Expr> i)
            : type(t), name(std::move(n)), init(std::move(i)){}
        const std::string &GetName() const{return name;}
        const Expr *GetInit() const{return init.get();}
        bool HasInit() const{return static_cast<bool>(init);}
        ExecResult Exec(ExecContext &) const override;
    };

    class AssignStmt final:public Stmt
    {
        std::string name;
        std::unique_ptr<Expr> value;

    public:
        AssignStmt(std::string n,std::unique_ptr<Expr> v)
            : name(std::move(n)), value(std::move(v)){}
        const std::string &GetName() const{return name;}
        const Expr *GetValue() const{return value.get();}
        ExecResult Exec(ExecContext &) const override;
    };

    class ExprStmt final:public Stmt
    {
        std::unique_ptr<Expr> expr;

    public:
        explicit ExprStmt(std::unique_ptr<Expr> e):expr(std::move(e)){}
        const Expr *GetExpr() const{return expr.get();}
        ExecResult Exec(ExecContext &) const override;
    };

    class ReturnStmt final:public Stmt
    {
        std::unique_ptr<Expr> expr;

    public:
        explicit ReturnStmt(std::unique_ptr<Expr> e):expr(std::move(e)){}
        const Expr *GetExpr() const{return expr.get();}
        bool HasExpr() const{return static_cast<bool>(expr);}
        ExecResult Exec(ExecContext &) const override;
    };

    class GotoStmt final:public Stmt
    {
        std::string label;

    public:
        explicit GotoStmt(std::string l):label(std::move(l)){}
        const std::string &GetLabel() const{return label;}
        ExecResult Exec(ExecContext &) const override;
    };

    class LabelStmt final:public Stmt
    {
        std::string label;

    public:
        explicit LabelStmt(std::string l):label(std::move(l)){}
        const std::string &GetLabel() const{return label;}
        ExecResult Exec(ExecContext &) const override;
    };

    class IfStmt final:public Stmt
    {
        std::unique_ptr<Expr> cond;
        std::unique_ptr<BlockStmt> then_block;
        std::unique_ptr<BlockStmt> else_block;

    public:
        IfStmt(std::unique_ptr<Expr> c,std::unique_ptr<BlockStmt> t,std::unique_ptr<BlockStmt> e)
            : cond(std::move(c)), then_block(std::move(t)), else_block(std::move(e)){}
        const Expr *GetCond() const{return cond.get();}
        const BlockStmt *GetThen() const{return then_block.get();}
        const BlockStmt *GetElse() const{return else_block.get();}
        ExecResult Exec(ExecContext &) const override;
    };

    class WhileStmt final:public Stmt
    {
        std::unique_ptr<Expr> cond;
        std::unique_ptr<BlockStmt> body;

    public:
        WhileStmt(std::unique_ptr<Expr> c,std::unique_ptr<BlockStmt> b)
            : cond(std::move(c)), body(std::move(b)){}
        const Expr *GetCond() const{return cond.get();}
        const BlockStmt *GetBody() const{return body.get();}
        ExecResult Exec(ExecContext &) const override;
    };

    class DoWhileStmt final:public Stmt
    {
        std::unique_ptr<BlockStmt> body;
        std::unique_ptr<Expr> cond;

    public:
        DoWhileStmt(std::unique_ptr<BlockStmt> b,std::unique_ptr<Expr> c)
            : body(std::move(b)), cond(std::move(c)){}
        const BlockStmt *GetBody() const{return body.get();}
        const Expr *GetCond() const{return cond.get();}
        ExecResult Exec(ExecContext &) const override;
    };

    class ForStmt final:public Stmt
    {
        std::unique_ptr<Stmt> init;
        std::unique_ptr<Expr> cond;
        std::unique_ptr<Expr> post;
        std::unique_ptr<BlockStmt> body;

    public:
        ForStmt(std::unique_ptr<Stmt> i,std::unique_ptr<Expr> c,std::unique_ptr<Expr> p,std::unique_ptr<BlockStmt> b)
            : init(std::move(i)), cond(std::move(c)), post(std::move(p)), body(std::move(b)){}
        const Stmt *GetInit() const{return init.get();}
        const Expr *GetCond() const{return cond.get();}
        const Expr *GetPost() const{return post.get();}
        const BlockStmt *GetBody() const{return body.get();}
        ExecResult Exec(ExecContext &) const override;
    };

    struct SwitchCase
    {
        std::unique_ptr<Expr> expr;
        std::unique_ptr<BlockStmt> block;
    };

    class SwitchStmt final:public Stmt
    {
        std::unique_ptr<Expr> expr;
        std::vector<SwitchCase> cases;
        std::unique_ptr<BlockStmt> default_block;

    public:
        SwitchStmt(std::unique_ptr<Expr> e,std::vector<SwitchCase> c,std::unique_ptr<BlockStmt> d)
            : expr(std::move(e)), cases(std::move(c)), default_block(std::move(d)){}

        const Expr *GetExpr() const{return expr.get();}
        const std::vector<SwitchCase> &GetCases() const{return cases;}
        const BlockStmt *GetDefault() const{return default_block.get();}
        ExecResult Exec(ExecContext &) const override;
    };

    class BreakStmt final:public Stmt
    {
    public:
        ExecResult Exec(ExecContext &) const override;
    };

    class ContinueStmt final:public Stmt
    {
    public:
        ExecResult Exec(ExecContext &) const override;
    };

    class EnumDeclStmt final:public Stmt
    {
    public:
        ExecResult Exec(ExecContext &) const override;
    };

    class BlockStmt final:public Stmt
    {
        std::vector<std::unique_ptr<Stmt>> statements;
        std::vector<SourceLocation> statement_locations;

    public:
        explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
            : statements(std::move(stmts)){}

        BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts,std::vector<SourceLocation> locations)
            : statements(std::move(stmts)), statement_locations(std::move(locations)){}

        const std::vector<std::unique_ptr<Stmt>> &GetStatements() const{return statements;}
        const std::vector<SourceLocation> &GetLocations() const{return statement_locations;}
        ExecResult Exec(ExecContext &) const override;
    };
}

