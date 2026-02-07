#include "DevilParse.h"
#include "DevilFunc.h"
#include <hgl/devil/DevilModule.h>
#include <cstring>
#include <ankerl/unordered_dense.h>

namespace hgl::devil
{
    namespace
    {
        void ConvertString(std::string &target,const char *source,int length)
        {
            std::string out;
            out.reserve(static_cast<size_t>(length));

            for(int i=0;i<length;i++)
            {
                if(source[i]=='\\' && i+1<length)
                {
                    const char esc=source[i+1];
                    switch(esc)
                    {
                        case 't': out.push_back('\t'); break;
                        case 'n': out.push_back('\n'); break;
                        case 'r': out.push_back('\r'); break;
                        case '\\': out.push_back('\\'); break;
                        case '\"': out.push_back('"'); break;
                        case '\'': out.push_back('\''); break;
                        default:
                            out.push_back('\\');
                            out.push_back(esc);
                            break;
                    }
                    i++;
                }
                else
                {
                    out.push_back(source[i]);
                }
            }

            target=std::move(out);
        }
    }

    Parse::Parse(Module *dm,const char *str,int len)
    {
        module=dm;
        source_start=str;
        source_cur=str;

        if(len==-1)
            source_length=strlen(str);
        else
            source_length=len;
    }

    eTokenType Parse::GetToken(std::string &intro)
    {
        while(true)
        {
            uint len=0;
            eTokenType type;
            const char *source;

            if(source_length<=0)
                return ttEnd;

            type=parse.GetToken(source_cur,source_length,&len);

            source=source_cur;
            source_cur+=len;
            source_length-=len;

            if(type<=ttEnd)
                return ttEnd;

            if(type<=ttMultilineComment)
                continue;

            intro.assign(source,static_cast<size_t>(len));
            return type;
        }
    }

    eTokenType Parse::CheckToken(std::string &intro)
    {
        const char *cur=source_cur;
        uint len=source_length;
        eTokenType type=GetToken(intro);

        source_cur=cur;
        source_length=len;

        return type;
    }

    bool Parse::GetToken(eTokenType tt,std::string &name)
    {
        eTokenType type;

        while(true)
        {
            type=GetToken(name);

            if(type==tt)
                return true;

            if(type<=ttEnd)
                return false;
        }
    }

    bool Parse::IsTypeToken(eTokenType type) const
    {
        return type==ttBool || type==ttString
            || type==ttInt || type==ttUInt
            || type==ttInt8 || type==ttUInt8
            || type==ttInt16 || type==ttUInt16
            || type==ttFloat;
    }

    int Parse::GetPrecedence(eTokenType type) const
    {
        switch(type)
        {
            case ttOr: return 1;
            case ttAnd: return 2;
            case ttEqual:
            case ttNotEqual: return 3;
            case ttLessThan:
            case ttLessThanOrEqual:
            case ttGreaterThan:
            case ttGreaterThanOrEqual: return 4;
            case ttPlus:
            case ttMinus: return 5;
            case ttStar:
            case ttSlash:
            case ttPercent: return 6;
            default: return 0;
        }
    }

    std::unique_ptr<Expr> Parse::ParsePrimary()
    {
        std::string name;
        eTokenType type=GetToken(name);

        if(type==ttIdentifier)
            return ParseCallOrIdentifier(name);

        if(type==ttIntConstant)
            return std::make_unique<LiteralExpr>(AstValue::MakeInt(static_cast<int32_t>(std::stoi(name))));

        if(type==ttFloatConstant || type==ttDoubleConstant)
            return std::make_unique<LiteralExpr>(AstValue::MakeFloat(static_cast<float>(std::stof(name))));

        if(type==ttTrue)
            return std::make_unique<LiteralExpr>(AstValue::MakeBool(true));

        if(type==ttFalse)
            return std::make_unique<LiteralExpr>(AstValue::MakeBool(false));

        if(type==ttStringConstant)
        {
            std::string str;
            ConvertString(str,name.c_str()+1,static_cast<int>(name.size())-2);
            return std::make_unique<LiteralExpr>(AstValue::MakeString(std::move(str)));
        }

        if(type==ttOpenParanthesis)
        {
            std::unique_ptr<Expr> expr=ParseExpression();
            GetToken(ttCloseParanthesis,name);
            return expr;
        }

        LogError("%s","ParsePrimary failed");
        return nullptr;
    }

    std::unique_ptr<Expr> Parse::ParseUnary()
    {
        std::string name;
        eTokenType type=CheckToken(name);

        if(type==ttMinus || type==ttPlus || type==ttNot)
        {
            GetToken(name);
            std::unique_ptr<Expr> right=ParseUnary();
            return std::make_unique<UnaryExpr>(type,std::move(right));
        }

        return ParsePrimary();
    }

    std::unique_ptr<Expr> Parse::ParseExpression(int min_prec)
    {
        std::unique_ptr<Expr> left=ParseUnary();
        if(!left)
            return nullptr;

        while(true)
        {
            std::string name;
            eTokenType op=CheckToken(name);
            const int prec=GetPrecedence(op);

            if(prec<min_prec || prec==0)
                break;

            GetToken(name);
            std::unique_ptr<Expr> right=ParseExpression(prec+1);
            if(!right)
                return nullptr;

            left=std::make_unique<BinaryExpr>(op,std::move(left),std::move(right));
        }

        return left;
    }

    std::unique_ptr<Expr> Parse::ParseCallOrIdentifier(const std::string &name)
    {
        std::string tmp;
        eTokenType next=CheckToken(tmp);

        if(next!=ttOpenParanthesis)
            return std::make_unique<IdentifierExpr>(name);

        GetToken(tmp); // '('
        std::vector<std::unique_ptr<Expr>> args;

        if(CheckToken(tmp)!=ttCloseParanthesis)
        {
            while(true)
            {
                std::unique_ptr<Expr> arg=ParseExpression();
                if(!arg)
                    return nullptr;
                args.push_back(std::move(arg));

                eTokenType sep=CheckToken(tmp);
                if(sep==ttListSeparator)
                {
                    GetToken(tmp);
                    continue;
                }
                break;
            }
        }

        GetToken(ttCloseParanthesis,tmp);
        return std::make_unique<CallExpr>(name,std::move(args));
    }

    std::unique_ptr<Stmt> Parse::ParseVarDecl(eTokenType type)
    {
        std::string name;
        if(GetToken(name)!=ttIdentifier)
        {
            LogError("%s","var decl missing identifier");
            return nullptr;
        }

        std::unique_ptr<Expr> init;
        std::string tmp;
        eTokenType next=CheckToken(tmp);
        if(next==ttAssignment)
        {
            GetToken(tmp);
            init=ParseExpression();
        }

        GetToken(ttEndStatement,tmp);
        return std::make_unique<VarDeclStmt>(type,name,std::move(init));
    }

    std::unique_ptr<Stmt> Parse::ParseIf()
    {
        std::string tmp;
        GetToken(ttOpenParanthesis,tmp);
        std::unique_ptr<Expr> cond=ParseExpression();
        GetToken(ttCloseParanthesis,tmp);

        std::unique_ptr<Stmt> then_stmt=ParseStatement(false);
        if(!then_stmt)
            return nullptr;

        std::unique_ptr<BlockStmt> then_block;
        if(auto *blk=dynamic_cast<BlockStmt *>(then_stmt.get()))
            then_block=std::unique_ptr<BlockStmt>(static_cast<BlockStmt *>(then_stmt.release()));
        else
        {
            std::vector<std::unique_ptr<Stmt>> stmts;
            stmts.push_back(std::move(then_stmt));
            then_block=std::make_unique<BlockStmt>(std::move(stmts));
        }

        std::unique_ptr<BlockStmt> else_block;
        eTokenType next=CheckToken(tmp);
        if(next==ttElse)
        {
            GetToken(tmp);
            std::unique_ptr<Stmt> else_stmt=ParseStatement(false);
            if(auto *blk=dynamic_cast<BlockStmt *>(else_stmt.get()))
                else_block=std::unique_ptr<BlockStmt>(static_cast<BlockStmt *>(else_stmt.release()));
            else
            {
                std::vector<std::unique_ptr<Stmt>> stmts;
                stmts.push_back(std::move(else_stmt));
                else_block=std::make_unique<BlockStmt>(std::move(stmts));
            }
        }

        return std::make_unique<IfStmt>(std::move(cond),std::move(then_block),std::move(else_block));
    }

    std::unique_ptr<Stmt> Parse::ParseWhile()
    {
        std::string tmp;
        GetToken(ttOpenParanthesis,tmp);
        std::unique_ptr<Expr> cond=ParseExpression();
        GetToken(ttCloseParanthesis,tmp);

        std::unique_ptr<Stmt> body_stmt=ParseStatement(false);
        if(!body_stmt)
            return nullptr;

        std::unique_ptr<BlockStmt> body;
        if(auto *blk=dynamic_cast<BlockStmt *>(body_stmt.get()))
            body=std::unique_ptr<BlockStmt>(static_cast<BlockStmt *>(body_stmt.release()));
        else
        {
            std::vector<std::unique_ptr<Stmt>> stmts;
            stmts.push_back(std::move(body_stmt));
            body=std::make_unique<BlockStmt>(std::move(stmts));
        }

        return std::make_unique<WhileStmt>(std::move(cond),std::move(body));
    }

    std::unique_ptr<Stmt> Parse::ParseFor()
    {
        std::string tmp;
        GetToken(ttOpenParanthesis,tmp);

        std::unique_ptr<Stmt> init;
        eTokenType next=CheckToken(tmp);
        if(next!=ttEndStatement)
        {
            if(IsTypeToken(next))
            {
                GetToken(tmp);
                init=ParseVarDecl(next);
            }
            else
            {
                std::unique_ptr<Expr> expr=ParseExpression();
                GetToken(ttEndStatement,tmp);
                init=std::make_unique<ExprStmt>(std::move(expr));
            }
        }
        else
        {
            GetToken(tmp);
        }

        std::unique_ptr<Expr> cond;
        next=CheckToken(tmp);
        if(next!=ttEndStatement)
            cond=ParseExpression();
        GetToken(ttEndStatement,tmp);

        std::unique_ptr<Expr> post;
        next=CheckToken(tmp);
        if(next!=ttCloseParanthesis)
            post=ParseExpression();
        GetToken(ttCloseParanthesis,tmp);

        std::unique_ptr<Stmt> body_stmt=ParseStatement(false);
        if(!body_stmt)
            return nullptr;

        std::unique_ptr<BlockStmt> body;
        if(auto *blk=dynamic_cast<BlockStmt *>(body_stmt.get()))
            body=std::unique_ptr<BlockStmt>(static_cast<BlockStmt *>(body_stmt.release()));
        else
        {
            std::vector<std::unique_ptr<Stmt>> stmts;
            stmts.push_back(std::move(body_stmt));
            body=std::make_unique<BlockStmt>(std::move(stmts));
        }

        return std::make_unique<ForStmt>(std::move(init),std::move(cond),std::move(post),std::move(body));
    }

    std::unique_ptr<Stmt> Parse::ParseSwitch()
    {
        std::string tmp;
        GetToken(ttOpenParanthesis,tmp);
        ParseExpression();
        GetToken(ttCloseParanthesis,tmp);
        GetToken(ttStartStatementBlock,tmp);
        int depth=1;
        while(depth>0)
        {
            eTokenType t=GetToken(tmp);
            if(t==ttStartStatementBlock)
                depth++;
            else
            if(t==ttEndStatementBlock)
                depth--;
            else
            if(t==ttEnd)
                break;
        }
        return std::make_unique<SwitchStmt>();
    }

    std::unique_ptr<Stmt> Parse::ParseEnum()
    {
        std::string tmp;
        GetToken(tmp); // enum name
        GetToken(ttStartStatementBlock,tmp);
        int depth=1;
        while(depth>0)
        {
            eTokenType t=GetToken(tmp);
            if(t==ttStartStatementBlock)
                depth++;
            else
            if(t==ttEndStatementBlock)
                depth--;
            else
            if(t==ttEnd)
                break;
        }
        if(CheckToken(tmp)==ttEndStatement)
            GetToken(tmp);
        return std::make_unique<EnumDeclStmt>();
    }

    std::unique_ptr<Stmt> Parse::ParseStatement(bool top_level)
    {
        std::string name;
        eTokenType type=CheckToken(name);

        if(type==ttStartStatementBlock)
            return ParseBlock(top_level);

        if(type==ttIf)
        {
            GetToken(name);
            return ParseIf();
        }

        if(type==ttWhile)
        {
            GetToken(name);
            return ParseWhile();
        }

        if(type==ttFor)
        {
            GetToken(name);
            return ParseFor();
        }

        if(type==ttSwitch)
        {
            GetToken(name);
            return ParseSwitch();
        }

        if(type==ttEnum)
        {
            GetToken(name);
            return ParseEnum();
        }

        if(type==ttReturn)
        {
            GetToken(name);
            if(CheckToken(name)==ttEndStatement)
            {
                GetToken(name);
                return std::make_unique<ReturnStmt>(nullptr);
            }

            std::unique_ptr<Expr> expr=ParseExpression();
            GetToken(ttEndStatement,name);
            return std::make_unique<ReturnStmt>(std::move(expr));
        }

        if(type==ttGoto)
        {
            GetToken(name);
            GetToken(name);
            GetToken(ttEndStatement,name);
            return std::make_unique<GotoStmt>(name);
        }

        if(IsTypeToken(type))
        {
            GetToken(name);
            return ParseVarDecl(type);
        }

        if(type==ttIdentifier)
        {
            GetToken(name);
            std::string next;
            eTokenType next_type=CheckToken(next);

            if(next_type==ttColon)
            {
                GetToken(next);
                if(!top_level)
                {
                    LogError("%s","label must be at top-level");
                    return nullptr;
                }
                return std::make_unique<LabelStmt>(name);
            }

            if(next_type==ttAssignment)
            {
                GetToken(next);
                std::unique_ptr<Expr> expr=ParseExpression();
                GetToken(ttEndStatement,next);
                return std::make_unique<AssignStmt>(name,std::move(expr));
            }

            if(next_type==ttOpenParanthesis)
            {
                std::unique_ptr<Expr> expr=ParseCallOrIdentifier(name);
                GetToken(ttEndStatement,next);
                return std::make_unique<ExprStmt>(std::move(expr));
            }
        }

        LogError("%s","ParseStatement failed");
        return nullptr;
    }

    std::unique_ptr<BlockStmt> Parse::ParseBlock(bool top_level)
    {
        std::string tmp;
        if(CheckToken(tmp)==ttStartStatementBlock)
            GetToken(tmp);

        std::vector<std::unique_ptr<Stmt>> stmts;

        while(true)
        {
            eTokenType t=CheckToken(tmp);
            if(t==ttEndStatementBlock)
            {
                GetToken(tmp);
                break;
            }
            if(t==ttEnd)
                break;

            std::unique_ptr<Stmt> stmt=ParseStatement(top_level);
            if(!stmt)
                return nullptr;
            stmts.push_back(std::move(stmt));
        }

        return std::make_unique<BlockStmt>(std::move(stmts));
    }

    bool Parse::ParseFunc(Func *func)
    {
        std::string name;

        GetToken(ttOpenParanthesis,name);
        if(CheckToken(name)!=ttCloseParanthesis)
        {
            LogError("%s","function parameters are not supported yet");
            return false;
        }
        GetToken(ttCloseParanthesis,name);

        std::unique_ptr<BlockStmt> body=ParseBlock(true);
        if(!body)
            return false;

        ankerl::unordered_dense::map<std::string,size_t> labels;
        const auto &stmts=body->GetStatements();
        for(size_t i=0;i<stmts.size();++i)
        {
            const auto *label_stmt=dynamic_cast<const LabelStmt *>(stmts[i].get());
            if(!label_stmt)
                continue;

            const std::string &label=label_stmt->GetLabel();
            if(labels.find(label)!=labels.end())
            {
                LogError("%s",("duplicate label: "+label).c_str());
                return false;
            }
            labels.emplace(label,i);
        }

        func->SetBody(std::move(body),std::move(labels));
        return true;
    }
}

