#include "DevilBytecodeBuilder.h"
#include <hgl/devil/DevilModule.h>

namespace hgl::devil
{
    int32_t BytecodeBuilder::AddLocal(const std::string &name)
    {
        const auto it=locals.find(name);
        if(it!=locals.end())
            return it->second;

        const int32_t index=static_cast<int32_t>(locals.size());
        locals.emplace(name,index);
        return index;
    }

    int32_t BytecodeBuilder::GetLocal(const std::string &name) const
    {
        const auto it=locals.find(name);
        if(it==locals.end())
            return -1;
        return it->second;
    }

    int32_t BytecodeBuilder::AddConst(BytecodeFunction &func,AstValue value)
    {
        func.constants.push_back(std::move(value));
        return static_cast<int32_t>(func.constants.size()-1);
    }

    size_t BytecodeBuilder::Emit(BytecodeFunction &func,OpCode op,int32_t a,int32_t b,int32_t c)
    {
        func.code.push_back(Instruction{op,a,b,c});
        return func.code.size()-1;
    }

    bool BytecodeBuilder::BuildExpr(BytecodeFunction &func,const Expr *expr)
    {
        if(!expr)
        {
            error="bytecode build missing expr";
            return false;
        }

        if(const auto *lit=dynamic_cast<const LiteralExpr *>(expr))
        {
            const int32_t idx=AddConst(func,lit->GetValue());
            Emit(func,OpCode::PushConst,idx);
            return true;
        }

        if(const auto *ident=dynamic_cast<const IdentifierExpr *>(expr))
        {
            const int32_t local=GetLocal(ident->GetName());
            if(local<0)
            {
                error="unknown local: "+ident->GetName();
                return false;
            }
            Emit(func,OpCode::LoadLocal,local);
            return true;
        }

        if(const auto *unary=dynamic_cast<const UnaryExpr *>(expr))
        {
            if(!BuildExpr(func,unary->GetExpr()))
                return false;

            switch(unary->GetOp())
            {
                case TokenType::Minus: Emit(func,OpCode::Neg); return true;
                case TokenType::Not: Emit(func,OpCode::Not); return true;
                case TokenType::BitNot: Emit(func,OpCode::BitNot); return true;
                case TokenType::Plus: return true;
                default: break;
            }

            error="bytecode build unsupported unary op";
            return false;
        }

        if(const auto *binary=dynamic_cast<const BinaryExpr *>(expr))
        {
            if(!BuildExpr(func,binary->GetLeft()))
                return false;
            if(!BuildExpr(func,binary->GetRight()))
                return false;

            switch(binary->GetOp())
            {
                case TokenType::Plus: Emit(func,OpCode::Add); return true;
                case TokenType::Minus: Emit(func,OpCode::Sub); return true;
                case TokenType::Star: Emit(func,OpCode::Mul); return true;
                case TokenType::Slash: Emit(func,OpCode::Div); return true;
                case TokenType::Percent: Emit(func,OpCode::Mod); return true;
                case TokenType::Equal: Emit(func,OpCode::Eq); return true;
                case TokenType::NotEqual: Emit(func,OpCode::Ne); return true;
                case TokenType::LessThan: Emit(func,OpCode::Lt); return true;
                case TokenType::LessThanOrEqual: Emit(func,OpCode::Le); return true;
                case TokenType::GreaterThan: Emit(func,OpCode::Gt); return true;
                case TokenType::GreaterThanOrEqual: Emit(func,OpCode::Ge); return true;
                case TokenType::And: Emit(func,OpCode::And); return true;
                case TokenType::Or: Emit(func,OpCode::Or); return true;
                case TokenType::BitOr: Emit(func,OpCode::BitOr); return true;
                case TokenType::BitXor: Emit(func,OpCode::BitXor); return true;
                case TokenType::Amp: Emit(func,OpCode::BitAnd); return true;
                case TokenType::BitShiftLeft: Emit(func,OpCode::Shl); return true;
                case TokenType::BitShiftRight:
                case TokenType::BitShiftRightArith:
                    Emit(func,OpCode::Shr);
                    return true;
                default: break;
            }

            error="bytecode build unsupported binary op";
            return false;
        }

        if(const auto *call=dynamic_cast<const CallExpr *>(expr))
        {
            const auto &args=call->GetArgs();
            for(const auto &arg:args)
            {
                if(!BuildExpr(func,arg.get()))
                    return false;
            }

            const int32_t name_index=AddConst(func,AstValue::MakeString(call->GetName()));
            const int32_t argc=static_cast<int32_t>(args.size());

            if(host_module && host_module->GetFuncMap(call->GetName()))
                Emit(func,OpCode::CallNative,name_index,argc);
            else
                Emit(func,OpCode::CallFunc,name_index,argc);

            return true;
        }

        error="bytecode build unsupported expr";
        return false;
    }

    bool BytecodeBuilder::BuildBlock(BytecodeFunction &func,const BlockStmt *block)
    {
        if(!block)
            return true;

        for(const auto &stmt:block->GetStatements())
        {
            if(!BuildStmt(func,stmt.get()))
                return false;
        }
        return true;
    }

    bool BytecodeBuilder::BuildStmt(BytecodeFunction &func,const Stmt *stmt)
    {
        if(!stmt)
            return true;

        if(const auto *block=dynamic_cast<const BlockStmt *>(stmt))
            return BuildBlock(func,block);

        if(const auto *var=dynamic_cast<const VarDeclStmt *>(stmt))
        {
            const int32_t local=AddLocal(var->GetName());
            if(var->HasInit())
            {
                if(!BuildExpr(func,var->GetInit()))
                    return false;
            }
            else
            {
                const int32_t idx=AddConst(func,AstValue::MakeVoid());
                Emit(func,OpCode::PushConst,idx);
            }
            Emit(func,OpCode::StoreLocal,local);
            return true;
        }

        if(const auto *assign=dynamic_cast<const AssignStmt *>(stmt))
        {
            const int32_t local=GetLocal(assign->GetName());
            if(local<0)
            {
                error="unknown local: "+assign->GetName();
                return false;
            }
            if(!BuildExpr(func,assign->GetValue()))
                return false;
            Emit(func,OpCode::StoreLocal,local);
            return true;
        }

        if(const auto *expr_stmt=dynamic_cast<const ExprStmt *>(stmt))
        {
            if(!BuildExpr(func,expr_stmt->GetExpr()))
                return false;
            Emit(func,OpCode::Pop);
            return true;
        }

        if(const auto *ret=dynamic_cast<const ReturnStmt *>(stmt))
        {
            if(ret->HasExpr())
            {
                if(!BuildExpr(func,ret->GetExpr()))
                    return false;
            }
            else
            {
                const int32_t idx=AddConst(func,AstValue::MakeVoid());
                Emit(func,OpCode::PushConst,idx);
            }
            Emit(func,OpCode::Ret);
            return true;
        }

        if(const auto *label=dynamic_cast<const LabelStmt *>(stmt))
        {
            labels[label->GetLabel()]=func.code.size();
            return true;
        }

        if(const auto *g=dynamic_cast<const GotoStmt *>(stmt))
        {
            const size_t jump_index=Emit(func,OpCode::Jump,-1);
            pending_gotos.push_back(PendingJump{jump_index,g->GetLabel()});
            return true;
        }

        if(const auto *ifs=dynamic_cast<const IfStmt *>(stmt))
        {
            if(!BuildExpr(func,ifs->GetCond()))
                return false;
            const size_t jmp_false=Emit(func,OpCode::JumpIfFalse,-1);
            if(!BuildBlock(func,ifs->GetThen()))
                return false;

            if(const auto *else_block=ifs->GetElse())
            {
                const size_t jmp_end=Emit(func,OpCode::Jump,-1);
                func.code[jmp_false].a=static_cast<int32_t>(func.code.size());
                if(!BuildBlock(func,else_block))
                    return false;
                func.code[jmp_end].a=static_cast<int32_t>(func.code.size());
            }
            else
            {
                func.code[jmp_false].a=static_cast<int32_t>(func.code.size());
            }
            return true;
        }

        if(const auto *ws=dynamic_cast<const WhileStmt *>(stmt))
        {
            const size_t loop_start=func.code.size();
            if(!BuildExpr(func,ws->GetCond()))
                return false;
            const size_t jmp_out=Emit(func,OpCode::JumpIfFalse,-1);
            if(!BuildBlock(func,ws->GetBody()))
                return false;
            Emit(func,OpCode::Jump,static_cast<int32_t>(loop_start));
            func.code[jmp_out].a=static_cast<int32_t>(func.code.size());
            return true;
        }

        error="bytecode build unsupported stmt";
        return false;
    }

    bool BytecodeBuilder::BuildFunction(const Func *func,BytecodeFunction &out_func)
    {
        if(!func)
        {
            error="bytecode build missing func";
            return false;
        }

        locals.clear();
        labels.clear();
        pending_gotos.clear();
        error.clear();

        out_func=BytecodeFunction{};
        out_func.name=func->func_name;

        if(!BuildBlock(out_func,func->GetBody()))
            return false;

        if(out_func.code.empty() || out_func.code.back().op!=OpCode::Ret)
        {
            const int32_t idx=AddConst(out_func,AstValue::MakeVoid());
            Emit(out_func,OpCode::PushConst,idx);
            Emit(out_func,OpCode::Ret);
        }

        for(const PendingJump &p:pending_gotos)
        {
            const auto it=labels.find(p.label);
            if(it==labels.end())
            {
                error="bytecode label not found: "+p.label;
                return false;
            }
            out_func.code[p.index].a=static_cast<int32_t>(it->second);
        }

        out_func.local_count=static_cast<int32_t>(locals.size());
        out_func.param_count=0;
        return true;
    }

    bool BytecodeBuilder::BuildAndAdd(const Func *func)
    {
        if(!bytecode_module)
        {
            error="bytecode output module not set";
            return false;
        }

        BytecodeFunction out_func;
        if(!BuildFunction(func,out_func))
            return false;

        return bytecode_module->AddFunction(std::move(out_func));
    }
}
