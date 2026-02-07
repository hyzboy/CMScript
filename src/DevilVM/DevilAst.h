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

    struct ExecContext
    {
        Module *module=nullptr;
        Context *context=nullptr;
        Func *func=nullptr;
        std::unordered_map<std::string,AstValue> locals;
        std::string error;
    };

    enum class ExecFlow
    {
        Normal,
        Return,
        Goto,
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

    class ForStmt final:public Stmt
    {
        std::unique_ptr<Stmt> init;
        std::unique_ptr<Expr> cond;
        std::unique_ptr<Expr> post;
        std::unique_ptr<BlockStmt> body;

    public:
        ForStmt(std::unique_ptr<Stmt> i,std::unique_ptr<Expr> c,std::unique_ptr<Expr> p,std::unique_ptr<BlockStmt> b)
            : init(std::move(i)), cond(std::move(c)), post(std::move(p)), body(std::move(b)){}
        ExecResult Exec(ExecContext &) const override;
    };

    class SwitchStmt final:public Stmt
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

    public:
        explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
            : statements(std::move(stmts)){}

        const std::vector<std::unique_ptr<Stmt>> &GetStatements() const{return statements;}
        ExecResult Exec(ExecContext &) const override;
    };
}

