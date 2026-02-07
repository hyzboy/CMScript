#include "DevilParse.h"
#include "DevilFunc.h"
#include <hgl/devil/DevilModule.h>
#include <cstring>
#include <ankerl/unordered_dense.h>

namespace hgl::devil
{
    namespace
    {
        struct SourceLocationInfo
        {
            int line=1;
            int column=1;
            std::string line_text;
            std::string caret_line;
        };

        SourceLocationInfo BuildSourceLocation(const char *source_start,const char *token_start)
        {
            SourceLocationInfo info;

            if(!source_start||!token_start||token_start<source_start)
                return info;

            const char *line_start=source_start;
            const char *p=source_start;

            while(p<token_start)
            {
                if(*p=='\r')
                {
                    info.line++;
                    if(p+1<token_start && p[1]=='\n')
                        ++p;
                    line_start=p+1;
                }
                else if(*p=='\n')
                {
                    info.line++;
                    line_start=p+1;
                }
                ++p;
            }

            info.column=int(token_start-line_start)+1;

            const char *line_end=token_start;
            while(*line_end && *line_end!='\r' && *line_end!='\n')
                ++line_end;

            info.line_text.assign(line_start,static_cast<size_t>(line_end-line_start));
            if(info.column<1)
                info.column=1;

            info.caret_line.assign(static_cast<size_t>(info.column-1),' ');
            info.caret_line.push_back('^');

            return info;
        }

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

    TokenType Parse::GetToken(std::string &intro)
    {
        while(true)
        {
            uint len=0;
            TokenType type;
            const char *source;

            if(source_length<=0)
                return TokenType::End;

            type=parse.GetToken(source_cur,source_length,&len);

            source=source_cur;
            source_cur+=len;
            source_length-=len;

            if(type<=TokenType::End)
                return TokenType::End;

            if(type<=TokenType::MultilineComment)
                continue;

            intro.assign(source,static_cast<size_t>(len));

            last_token_start=source;
            last_token_length=len;
            last_token_type=type;

            return type;
        }
    }

    TokenType Parse::CheckToken(std::string &intro)
    {
        const char *cur=source_cur;
        uint len=source_length;
        TokenType type=GetToken(intro);

        source_cur=cur;
        source_length=len;

        return type;
    }

    bool Parse::GetToken(TokenType tt,std::string &name)
    {
        TokenType type;

        while(true)
        {
            type=GetToken(name);

            if(type==tt)
                return true;

            if(type<=TokenType::End)
                return false;
        }
    }

    bool Parse::IsTypeToken(TokenType type) const
    {
        return type==TokenType::Bool || type==TokenType::String
            || type==TokenType::Int || type==TokenType::UInt
            || type==TokenType::Int8 || type==TokenType::UInt8
            || type==TokenType::Int16 || type==TokenType::UInt16
            || type==TokenType::Float;
    }

    int Parse::GetPrecedence(TokenType type) const
    {
        switch(type)
        {
            case TokenType::Or: return 1;
            case TokenType::And: return 2;
            case TokenType::BitOr: return 3;
            case TokenType::BitXor: return 4;
            case TokenType::Amp: return 5;
            case TokenType::Equal:
            case TokenType::NotEqual: return 6;
            case TokenType::LessThan:
            case TokenType::LessThanOrEqual:
            case TokenType::GreaterThan:
            case TokenType::GreaterThanOrEqual: return 7;
            case TokenType::BitShiftLeft:
            case TokenType::BitShiftRight:
            case TokenType::BitShiftRightArith: return 8;
            case TokenType::Plus:
            case TokenType::Minus: return 9;
            case TokenType::Star:
            case TokenType::Slash:
            case TokenType::Percent: return 10;
            default: return 0;
        }
    }

    std::unique_ptr<Expr> Parse::ParsePrimary()
    {
        std::string name;
        TokenType type=GetToken(name);

        if(type==TokenType::Cast)
        {
            SourceLocationInfo loc=BuildSourceLocation(source_start,last_token_start?last_token_start:source_cur);
            LogWarning("%s",("explicit cast used at "
                +std::to_string(loc.line)+":"+std::to_string(loc.column)
                +"\n"+loc.line_text+"\n"+loc.caret_line).c_str());

            std::string tmp;
            GetToken(TokenType::OpenParanthesis,tmp);
            std::string type_name;
            const TokenType target_type=GetToken(type_name);
            if(!IsTypeToken(target_type))
            {
                LogError("%s","cast target must be a type");
                return nullptr;
            }
            GetToken(TokenType::ListSeparator,tmp);
            std::unique_ptr<Expr> expr=ParseExpression();
            if(!expr)
                return nullptr;
            GetToken(TokenType::CloseParanthesis,tmp);
            return std::make_unique<CastExpr>(target_type,std::move(expr));
        }

        if(type==TokenType::Identifier)
            return ParseCallOrIdentifier(name);

        if(type==TokenType::IntConstant)
            return std::make_unique<LiteralExpr>(AstValue::MakeInt(static_cast<int32_t>(std::stoi(name))));

        if(type==TokenType::FloatConstant)
            return std::make_unique<LiteralExpr>(AstValue::MakeFloat(static_cast<float>(std::stof(name))));

        if(type==TokenType::DoubleConstant)
            return std::make_unique<LiteralExpr>(AstValue::MakeDouble(static_cast<double>(std::stod(name))));

        if(type==TokenType::True)
            return std::make_unique<LiteralExpr>(AstValue::MakeBool(true));

        if(type==TokenType::False)
            return std::make_unique<LiteralExpr>(AstValue::MakeBool(false));

        if(type==TokenType::StringConstant)
        {
            std::string str;
            ConvertString(str,name.c_str()+1,static_cast<int>(name.size())-2);
            return std::make_unique<LiteralExpr>(AstValue::MakeString(std::move(str)));
        }

        if(type==TokenType::OpenParanthesis)
        {
            const char *paren_start=last_token_start;
            const char *saved_cur=source_cur;
            uint saved_len=source_length;

            std::string tmp;
            TokenType next=GetToken(tmp);
            if(IsTypeToken(next) && CheckToken(name)==TokenType::CloseParanthesis)
            {
                GetToken(TokenType::CloseParanthesis,name);
                SourceLocationInfo loc=BuildSourceLocation(source_start,paren_start?paren_start:source_cur);
                LogWarning("%s",("explicit cast used at "
                    +std::to_string(loc.line)+":"+std::to_string(loc.column)
                    +"\n"+loc.line_text+"\n"+loc.caret_line).c_str());

                std::unique_ptr<Expr> expr=ParseUnary();
                if(!expr)
                    return nullptr;
                return std::make_unique<CastExpr>(next,std::move(expr));
            }

            source_cur=saved_cur;
            source_length=saved_len;

            std::unique_ptr<Expr> expr=ParseExpression();
            GetToken(TokenType::CloseParanthesis,name);
            return expr;
        }

        LogError("%s","ParsePrimary failed");
        return nullptr;
    }

    std::unique_ptr<Expr> Parse::ParseUnary()
    {
        std::string name;
        TokenType type=CheckToken(name);

        if(type==TokenType::Minus || type==TokenType::Plus || type==TokenType::Not)
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

        if(min_prec<=0)
        {
            std::string name;
            TokenType next=CheckToken(name);
            if(next==TokenType::Assignment)
            {
                auto *ident=dynamic_cast<IdentifierExpr *>(left.get());
                if(!ident)
                {
                    LogError("%s","assignment target must be identifier");
                    return nullptr;
                }
                GetToken(name);
                std::unique_ptr<Expr> rhs=ParseExpression(0);
                if(!rhs)
                    return nullptr;
                const std::string ident_name=ident->GetName();
                return std::make_unique<AssignExpr>(ident_name,std::move(rhs));
            }
        }

        while(true)
        {
            std::string name;
            TokenType op=CheckToken(name);
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
        TokenType next=CheckToken(tmp);

        if(next!=TokenType::OpenParanthesis)
            return std::make_unique<IdentifierExpr>(name);

        GetToken(tmp); // '('
        std::vector<std::unique_ptr<Expr>> args;

        if(CheckToken(tmp)!=TokenType::CloseParanthesis)
        {
            while(true)
            {
                std::unique_ptr<Expr> arg=ParseExpression();
                if(!arg)
                    return nullptr;
                args.push_back(std::move(arg));

                TokenType sep=CheckToken(tmp);
                if(sep==TokenType::ListSeparator)
                {
                    GetToken(tmp);
                    continue;
                }
                break;
            }
        }

        GetToken(TokenType::CloseParanthesis,tmp);
        return std::make_unique<CallExpr>(name,std::move(args));
    }

    std::unique_ptr<Stmt> Parse::ParseVarDecl(TokenType type)
    {
        std::string name;
        if(GetToken(name)!=TokenType::Identifier)
        {
            LogError("%s","var decl missing identifier");
            return nullptr;
        }

        std::unique_ptr<Expr> init;
        std::string tmp;
        TokenType next=CheckToken(tmp);
        if(next==TokenType::Assignment)
        {
            GetToken(tmp);
            init=ParseExpression();
        }

        GetToken(TokenType::EndStatement,tmp);
        return std::make_unique<VarDeclStmt>(type,name,std::move(init));
    }

    std::unique_ptr<Stmt> Parse::ParseIf()
    {
        std::string tmp;
        GetToken(TokenType::OpenParanthesis,tmp);
        std::unique_ptr<Expr> cond=ParseExpression();
        GetToken(TokenType::CloseParanthesis,tmp);

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
            std::vector<SourceLocation> locations;
            locations.push_back(GetLastStatementLocation());
            then_block=std::make_unique<BlockStmt>(std::move(stmts),std::move(locations));
        }

        std::unique_ptr<BlockStmt> else_block;
        TokenType next=CheckToken(tmp);
        if(next==TokenType::Else)
        {
            GetToken(tmp);
            std::unique_ptr<Stmt> else_stmt=ParseStatement(false);
            if(auto *blk=dynamic_cast<BlockStmt *>(else_stmt.get()))
                else_block=std::unique_ptr<BlockStmt>(static_cast<BlockStmt *>(else_stmt.release()));
            else
            {
                std::vector<std::unique_ptr<Stmt>> stmts;
                stmts.push_back(std::move(else_stmt));
                std::vector<SourceLocation> locations;
                locations.push_back(GetLastStatementLocation());
                else_block=std::make_unique<BlockStmt>(std::move(stmts),std::move(locations));
            }
        }

        return std::make_unique<IfStmt>(std::move(cond),std::move(then_block),std::move(else_block));
    }

    std::unique_ptr<Stmt> Parse::ParseWhile()
    {
        std::string tmp;
        GetToken(TokenType::OpenParanthesis,tmp);
        std::unique_ptr<Expr> cond=ParseExpression();
        GetToken(TokenType::CloseParanthesis,tmp);

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
            std::vector<SourceLocation> locations;
            locations.push_back(GetLastStatementLocation());
            body=std::make_unique<BlockStmt>(std::move(stmts),std::move(locations));
        }

        return std::make_unique<WhileStmt>(std::move(cond),std::move(body));
    }

    std::unique_ptr<Stmt> Parse::ParseDoWhile()
    {
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
            std::vector<SourceLocation> locations;
            locations.push_back(GetLastStatementLocation());
            body=std::make_unique<BlockStmt>(std::move(stmts),std::move(locations));
        }

        std::string tmp;
        if(GetToken(tmp)!=TokenType::While)
        {
            LogError("%s","do-while missing while keyword");
            return nullptr;
        }

        GetToken(TokenType::OpenParanthesis,tmp);
        std::unique_ptr<Expr> cond=ParseExpression();
        GetToken(TokenType::CloseParanthesis,tmp);
        GetToken(TokenType::EndStatement,tmp);

        return std::make_unique<DoWhileStmt>(std::move(body),std::move(cond));
    }

    std::unique_ptr<Stmt> Parse::ParseFor()
    {
        std::string tmp;
        GetToken(TokenType::OpenParanthesis,tmp);

        std::unique_ptr<Stmt> init;
        TokenType next=CheckToken(tmp);
        if(next!=TokenType::EndStatement)
        {
            if(IsTypeToken(next))
            {
                GetToken(tmp);
                init=ParseVarDecl(next);
            }
            else
            {
                std::unique_ptr<Expr> expr=ParseExpression();
                GetToken(TokenType::EndStatement,tmp);
                init=std::make_unique<ExprStmt>(std::move(expr));
            }
        }
        else
        {
            GetToken(tmp);
        }

        std::unique_ptr<Expr> cond;
        next=CheckToken(tmp);
        if(next!=TokenType::EndStatement)
            cond=ParseExpression();
        GetToken(TokenType::EndStatement,tmp);

        std::unique_ptr<Expr> post;
        next=CheckToken(tmp);
        if(next!=TokenType::CloseParanthesis)
            post=ParseExpression();
        GetToken(TokenType::CloseParanthesis,tmp);

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
            std::vector<SourceLocation> locations;
            locations.push_back(GetLastStatementLocation());
            body=std::make_unique<BlockStmt>(std::move(stmts),std::move(locations));
        }

        return std::make_unique<ForStmt>(std::move(init),std::move(cond),std::move(post),std::move(body));
    }

    std::unique_ptr<Stmt> Parse::ParseSwitch()
    {
        std::string tmp;
        GetToken(TokenType::OpenParanthesis,tmp);
        std::unique_ptr<Expr> expr=ParseExpression();
        GetToken(TokenType::CloseParanthesis,tmp);
        GetToken(TokenType::StartStatementBlock,tmp);

        std::vector<SwitchCase> cases;
        std::unique_ptr<BlockStmt> default_block;

        while(true)
        {
            TokenType t=CheckToken(tmp);
            if(t==TokenType::EndStatementBlock)
            {
                GetToken(tmp);
                break;
            }
            if(t==TokenType::End)
                break;

            if(t==TokenType::Case)
            {
                GetToken(tmp);
                std::unique_ptr<Expr> case_expr=ParseExpression();
                GetToken(TokenType::Colon,tmp);

                std::vector<std::unique_ptr<Stmt>> stmts;
                std::vector<SourceLocation> locations;
                while(true)
                {
                    TokenType nt=CheckToken(tmp);
                    if(nt==TokenType::Case || nt==TokenType::Default || nt==TokenType::EndStatementBlock || nt==TokenType::End)
                        break;

                    std::unique_ptr<Stmt> stmt=ParseStatement(false);
                    if(!stmt)
                        return nullptr;
                    stmts.push_back(std::move(stmt));
                    locations.push_back(GetLastStatementLocation());
                }

                cases.push_back(SwitchCase{std::move(case_expr),std::make_unique<BlockStmt>(std::move(stmts),std::move(locations))});
                continue;
            }

            if(t==TokenType::Default)
            {
                GetToken(tmp);
                GetToken(TokenType::Colon,tmp);

                std::vector<std::unique_ptr<Stmt>> stmts;
                std::vector<SourceLocation> locations;
                while(true)
                {
                    TokenType nt=CheckToken(tmp);
                    if(nt==TokenType::Case || nt==TokenType::Default || nt==TokenType::EndStatementBlock || nt==TokenType::End)
                        break;

                    std::unique_ptr<Stmt> stmt=ParseStatement(false);
                    if(!stmt)
                        return nullptr;
                    stmts.push_back(std::move(stmt));
                    locations.push_back(GetLastStatementLocation());
                }

                default_block=std::make_unique<BlockStmt>(std::move(stmts),std::move(locations));
                continue;
            }

            LogError("%s","ParseSwitch unexpected token");
            return nullptr;
        }

        return std::make_unique<SwitchStmt>(std::move(expr),std::move(cases),std::move(default_block));
    }

    std::unique_ptr<Stmt> Parse::ParseEnum()
    {
        std::string tmp;
        GetToken(tmp); // enum name
        GetToken(TokenType::StartStatementBlock,tmp);
        int depth=1;
        while(depth>0)
        {
            TokenType t=GetToken(tmp);
            if(t==TokenType::StartStatementBlock)
                depth++;
            else
            if(t==TokenType::EndStatementBlock)
                depth--;
            else
            if(t==TokenType::End)
                break;
        }
        if(CheckToken(tmp)==TokenType::EndStatement)
            GetToken(tmp);
        return std::make_unique<EnumDeclStmt>();
    }

    std::unique_ptr<Stmt> Parse::ParseStatement(bool top_level)
    {
        std::string name;
        TokenType type=CheckToken(name);

        {
            SourceLocationInfo loc=BuildSourceLocation(source_start,source_cur);
            last_stmt_loc.line=loc.line;
            last_stmt_loc.column=loc.column;
            last_stmt_loc.line_text=loc.line_text;
            last_stmt_loc.caret_line=loc.caret_line;
        }

        if(type==TokenType::StartStatementBlock)
            return ParseBlock(top_level);

        if(type==TokenType::If)
        {
            GetToken(name);
            return ParseIf();
        }

        if(type==TokenType::While)
        {
            GetToken(name);
            return ParseWhile();
        }

        if(type==TokenType::Do)
        {
            GetToken(name);
            return ParseDoWhile();
        }

        if(type==TokenType::For)
        {
            GetToken(name);
            return ParseFor();
        }

        if(type==TokenType::Switch)
        {
            GetToken(name);
            return ParseSwitch();
        }

        if(type==TokenType::Enum)
        {
            GetToken(name);
            return ParseEnum();
        }

        if(type==TokenType::Return)
        {
            GetToken(name);
            if(CheckToken(name)==TokenType::EndStatement)
            {
                GetToken(name);
                return std::make_unique<ReturnStmt>(nullptr);
            }

            std::unique_ptr<Expr> expr=ParseExpression();
            GetToken(TokenType::EndStatement,name);
            return std::make_unique<ReturnStmt>(std::move(expr));
        }

        if(type==TokenType::Goto)
        {
            GetToken(name);
            std::string label;
            GetToken(label);
            GetToken(TokenType::EndStatement,name);
            return std::make_unique<GotoStmt>(label);
        }

        if(type==TokenType::Break)
        {
            GetToken(name);
            GetToken(TokenType::EndStatement,name);
            return std::make_unique<BreakStmt>();
        }

        if(type==TokenType::Continue)
        {
            GetToken(name);
            GetToken(TokenType::EndStatement,name);
            return std::make_unique<ContinueStmt>();
        }

        if(IsTypeToken(type))
        {
            GetToken(name);
            return ParseVarDecl(type);
        }

        if(type==TokenType::Identifier)
        {
            GetToken(name);
            std::string next;
            TokenType next_type=CheckToken(next);

            if(next_type==TokenType::Colon)
            {
                GetToken(next);
                if(!top_level)
                {
                    LogError("%s","label must be at top-level");
                    return nullptr;
                }
                if(CheckToken(next)==TokenType::EndStatement)
                    GetToken(next);
                return std::make_unique<LabelStmt>(name);
            }

            if(next_type==TokenType::Assignment)
            {
                GetToken(next);
                std::unique_ptr<Expr> expr=ParseExpression();
                GetToken(TokenType::EndStatement,next);
                return std::make_unique<AssignStmt>(name,std::move(expr));
            }

            if(next_type==TokenType::OpenParanthesis)
            {
                std::unique_ptr<Expr> expr=ParseCallOrIdentifier(name);
                GetToken(TokenType::EndStatement,next);
                return std::make_unique<ExprStmt>(std::move(expr));
            }

            SourceLocationInfo loc=BuildSourceLocation(source_start,last_token_start);
            LogError("%s",("ParseStatement unexpected identifier usage: token="
                +std::string(asGetTokenDefinition(type))
                +" intro='"+name+"' next="
                +std::string(asGetTokenDefinition(next_type))
                +" next_intro='"+next+"' top_level="+(top_level?"1":"0")
                +" at "+std::to_string(loc.line)+":"+std::to_string(loc.column)
                +"\n"+loc.line_text+"\n"+loc.caret_line).c_str());
        }

        std::string next_intro;
        TokenType next_type=CheckToken(next_intro);
        const size_t preview_len=source_length<80?source_length:80;
        const std::string preview=preview_len>0?std::string(source_cur,preview_len):std::string();
        SourceLocationInfo loc=BuildSourceLocation(source_start,last_token_start?last_token_start:source_cur);

        LogError("%s",("ParseStatement failed: token="
            +std::string(asGetTokenDefinition(type))
            +" intro='"+name+"' next="
            +std::string(asGetTokenDefinition(next_type))
            +" next_intro='"+next_intro+"' top_level="+(top_level?"1":"0")
            +" remaining="+std::to_string(source_length)
            +" preview='"+preview+"'"
            +" at "+std::to_string(loc.line)+":"+std::to_string(loc.column)
            +"\n"+loc.line_text+"\n"+loc.caret_line).c_str());
        return nullptr;
    }

    std::unique_ptr<BlockStmt> Parse::ParseBlock(bool top_level)
    {
        std::string tmp;
        if(CheckToken(tmp)==TokenType::StartStatementBlock)
            GetToken(tmp);

        std::vector<std::unique_ptr<Stmt>> stmts;
        std::vector<SourceLocation> locations;

        while(true)
        {
            TokenType t=CheckToken(tmp);
            if(t==TokenType::EndStatementBlock)
            {
                GetToken(tmp);
                break;
            }
            if(t==TokenType::End)
                break;

            std::unique_ptr<Stmt> stmt=ParseStatement(top_level);
            if(!stmt)
                return nullptr;
            stmts.push_back(std::move(stmt));
            locations.push_back(GetLastStatementLocation());
        }

        return std::make_unique<BlockStmt>(std::move(stmts),std::move(locations));
    }

    bool Parse::ParseFunc(Func *func)
    {
        std::string name;

        GetToken(TokenType::OpenParanthesis,name);
        std::vector<Func::Param> params;
        if(CheckToken(name)!=TokenType::CloseParanthesis)
        {
            while(true)
            {
                std::string type_name;
                const TokenType type=GetToken(type_name);
                if(!IsTypeToken(type))
                {
                    SourceLocationInfo loc=BuildSourceLocation(source_start,last_token_start?last_token_start:source_cur);
                    LogError("%s",("function param type expected at "
                        +std::to_string(loc.line)+":"+std::to_string(loc.column)
                        +" got="+std::string(asGetTokenDefinition(type))
                        +" intro='"+type_name+"'\n"
                        +loc.line_text+"\n"+loc.caret_line).c_str());
                    return false;
                }

                std::string param_name;
                if(GetToken(param_name)!=TokenType::Identifier)
                {
                    SourceLocationInfo loc=BuildSourceLocation(source_start,last_token_start?last_token_start:source_cur);
                    LogError("%s",("function param name expected at "
                        +std::to_string(loc.line)+":"+std::to_string(loc.column)
                        +" intro='"+param_name+"'\n"
                        +loc.line_text+"\n"+loc.caret_line).c_str());
                    return false;
                }

                bool dup=false;
                for(const auto &p:params)
                {
                    if(p.name==param_name)
                    {
                        dup=true;
                        break;
                    }
                }
                if(dup)
                {
                    LogError("%s",("duplicate function param: "+param_name).c_str());
                    return false;
                }

                params.push_back(Func::Param{type,param_name});

                TokenType sep=CheckToken(name);
                if(sep==TokenType::ListSeparator)
                {
                    GetToken(name);
                    continue;
                }
                break;
            }
        }
        GetToken(TokenType::CloseParanthesis,name);

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

        func->SetParams(std::move(params));
        func->SetBody(std::move(body),std::move(labels));
        return true;
    }
}



