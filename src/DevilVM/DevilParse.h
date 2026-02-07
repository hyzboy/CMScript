#pragma once

#include "as_tokenizer.h"
#include "DevilAst.h"
#include <string>
#include <hgl/platform/compiler/EventFunc.h>
#include <hgl/log/Log.h>

namespace hgl::devil
{
    class Parse
    {
        OBJECT_LOGGER

        Module *module;

        const char *source_start;
        const char *source_cur;
        uint source_length;

        const char *last_token_start=nullptr;
        uint last_token_length=0;
        TokenType last_token_type=TokenType::End;

        asCTokenizer parse;

    private:
        std::unique_ptr<BlockStmt> ParseBlock(bool top_level);
        std::unique_ptr<Stmt> ParseStatement(bool top_level);
        std::unique_ptr<Stmt> ParseVarDecl(TokenType type);
        std::unique_ptr<Expr> ParseExpression(int min_prec=0);
        std::unique_ptr<Expr> ParseUnary();
        std::unique_ptr<Expr> ParsePrimary();
        std::unique_ptr<Expr> ParseCallOrIdentifier(const std::string &name);
        std::unique_ptr<Stmt> ParseIf();
        std::unique_ptr<Stmt> ParseWhile();
        std::unique_ptr<Stmt> ParseFor();
        std::unique_ptr<Stmt> ParseSwitch();
        std::unique_ptr<Stmt> ParseEnum();

        bool IsTypeToken(TokenType) const;
        int GetPrecedence(TokenType) const;

    public:
        Parse(Module *,const char *,int=-1);

        TokenType GetToken(std::string &);     //取得一个token,自动跳过注释、换行、空格
        TokenType CheckToken(std::string &);   //检测下一个token,自动跳过注释、换行、空格,但不取出

        bool GetToken(TokenType,std::string &);    //找某一种Token为止

        bool ParseFunc(Func *);        //解析一个函数
    };
}//namespace hgl::devil
