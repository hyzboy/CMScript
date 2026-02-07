#include "DevilBytecodeBuilder.h"
#include <hgl/devil/DevilModule.h>

namespace hgl::devil
{
    namespace
    {
        bool ComputeFastInt(BytecodeFunction &func)
        {
            func.const_ints.clear();

            for(const AstValue &value:func.constants)
            {
                if(value.type!=TokenType::Bool && value.type!=TokenType::Int
                    && value.type!=TokenType::Int8 && value.type!=TokenType::Int16)
                {
                    return false;
                }
                func.const_ints.push_back(value.ToInt());
            }

            for(const Instruction &ins:func.code)
            {
                switch(ins.op)
                {
                    case OpCode::Nop:
                    case OpCode::PushConst:
                    case OpCode::Pop:
                    case OpCode::LoadLocal:
                    case OpCode::StoreLocal:
                    case OpCode::AddLocalConst:
                    case OpCode::SubLocalConst:
                    case OpCode::MulLocalConst:
                    case OpCode::DivLocalConst:
                    case OpCode::ModLocalConst:
                    case OpCode::BitAndLocalConst:
                    case OpCode::BitOrLocalConst:
                    case OpCode::BitXorLocalConst:
                    case OpCode::ShlLocalConst:
                    case OpCode::ShrLocalConst:
                    case OpCode::AddLocalLocal:
                    case OpCode::SubLocalLocal:
                    case OpCode::MulLocalLocal:
                    case OpCode::DivLocalLocal:
                    case OpCode::ModLocalLocal:
                    case OpCode::BitAndLocalLocal:
                    case OpCode::BitOrLocalLocal:
                    case OpCode::BitXorLocalLocal:
                    case OpCode::ShlLocalLocal:
                    case OpCode::ShrLocalLocal:
                    case OpCode::JumpIfLocalGeConst:
                    case OpCode::JumpIfLocalGtConst:
                    case OpCode::JumpIfLocalLeConst:
                    case OpCode::JumpIfLocalLtConst:
                    case OpCode::JumpIfLocalEqConst:
                    case OpCode::JumpIfLocalNeConst:
                    case OpCode::JumpIfLocalGeLocal:
                    case OpCode::JumpIfLocalGtLocal:
                    case OpCode::JumpIfLocalLeLocal:
                    case OpCode::JumpIfLocalLtLocal:
                    case OpCode::JumpIfLocalEqLocal:
                    case OpCode::JumpIfLocalNeLocal:
                    case OpCode::Add:
                    case OpCode::Sub:
                    case OpCode::Mul:
                    case OpCode::Div:
                    case OpCode::Mod:
                    case OpCode::Neg:
                    case OpCode::Not:
                    case OpCode::BitNot:
                    case OpCode::BitAnd:
                    case OpCode::BitOr:
                    case OpCode::BitXor:
                    case OpCode::Shl:
                    case OpCode::Shr:
                    case OpCode::Eq:
                    case OpCode::Ne:
                    case OpCode::Lt:
                    case OpCode::Le:
                    case OpCode::Gt:
                    case OpCode::Ge:
                    case OpCode::And:
                    case OpCode::Or:
                    case OpCode::Jump:
                    case OpCode::JumpIfFalse:
                    case OpCode::Ret:
                        break;
                    case OpCode::Cast:
                    {
                        const TokenType target=static_cast<TokenType>(ins.a);
                        if(target!=TokenType::Bool && target!=TokenType::Int
                            && target!=TokenType::Int8 && target!=TokenType::Int16)
                            return false;
                        break;
                    }
                    default:
                        return false;
                }
            }

            return true;
        }

        bool ComputeFastFloat(BytecodeFunction &func)
        {
            func.const_floats.clear();
            bool has_double=false;

            for(const AstValue &value:func.constants)
            {
                switch(value.type)
                {
                    case TokenType::Bool:
                    case TokenType::Int:
                    case TokenType::Int8:
                    case TokenType::Int16:
                    case TokenType::UInt:
                    case TokenType::UInt8:
                    case TokenType::UInt16:
                    case TokenType::Float:
                    case TokenType::Double:
                        func.const_floats.push_back(value.ToDouble());
                        if(value.type==TokenType::Double)
                            has_double=true;
                        break;
                    default:
                        return false;
                }
            }

            for(const Instruction &ins:func.code)
            {
                switch(ins.op)
                {
                    case OpCode::Nop:
                    case OpCode::PushConst:
                    case OpCode::Pop:
                    case OpCode::LoadLocal:
                    case OpCode::StoreLocal:
                    case OpCode::AddLocalConst:
                    case OpCode::SubLocalConst:
                    case OpCode::MulLocalConst:
                    case OpCode::DivLocalConst:
                    case OpCode::ModLocalConst:
                    case OpCode::AddLocalLocal:
                    case OpCode::SubLocalLocal:
                    case OpCode::MulLocalLocal:
                    case OpCode::DivLocalLocal:
                    case OpCode::ModLocalLocal:
                    case OpCode::JumpIfLocalGeConst:
                    case OpCode::JumpIfLocalGtConst:
                    case OpCode::JumpIfLocalLeConst:
                    case OpCode::JumpIfLocalLtConst:
                    case OpCode::JumpIfLocalEqConst:
                    case OpCode::JumpIfLocalNeConst:
                    case OpCode::JumpIfLocalGeLocal:
                    case OpCode::JumpIfLocalGtLocal:
                    case OpCode::JumpIfLocalLeLocal:
                    case OpCode::JumpIfLocalLtLocal:
                    case OpCode::JumpIfLocalEqLocal:
                    case OpCode::JumpIfLocalNeLocal:
                    case OpCode::Add:
                    case OpCode::Sub:
                    case OpCode::Mul:
                    case OpCode::Div:
                    case OpCode::Mod:
                    case OpCode::Neg:
                    case OpCode::Not:
                    case OpCode::Eq:
                    case OpCode::Ne:
                    case OpCode::Lt:
                    case OpCode::Le:
                    case OpCode::Gt:
                    case OpCode::Ge:
                    case OpCode::And:
                    case OpCode::Or:
                    case OpCode::Jump:
                    case OpCode::JumpIfFalse:
                    case OpCode::Ret:
                        break;
                    case OpCode::Cast:
                    {
                        const TokenType target=static_cast<TokenType>(ins.a);
                        if(target==TokenType::Double)
                        {
                            has_double=true;
                            break;
                        }
                        if(target!=TokenType::Bool && target!=TokenType::Float)
                            return false;
                        break;
                    }
                    default:
                        return false;
                }
            }

            func.fast_double=has_double;
            return true;
        }

        void PeepholeFused(BytecodeFunction &func)
        {
            auto &code=func.code;
            if(code.size()<4)
                return;

            auto fuse_local_const=[&](size_t i,OpCode fused)
            {
                const Instruction &a=code[i];
                const Instruction &b=code[i+1];
                const Instruction &d=code[i+3];
                if(a.op==OpCode::LoadLocal && b.op==OpCode::PushConst
                    && d.op==OpCode::StoreLocal && a.a==d.a)
                {
                    code[i]=Instruction{fused,a.a,b.a,0};
                    code[i+1].op=OpCode::Nop;
                    code[i+2].op=OpCode::Nop;
                    code[i+3].op=OpCode::Nop;
                    return true;
                }
                return false;
            };

            auto fuse_local_cmp=[&](size_t i,OpCode fused)
            {
                const Instruction &a=code[i];
                const Instruction &b=code[i+1];
                const Instruction &d=code[i+3];
                if(a.op==OpCode::LoadLocal && b.op==OpCode::PushConst
                    && d.op==OpCode::JumpIfFalse)
                {
                    code[i]=Instruction{fused,a.a,b.a,d.a};
                    code[i+1].op=OpCode::Nop;
                    code[i+2].op=OpCode::Nop;
                    code[i+3].op=OpCode::Nop;
                    return true;
                }
                return false;
            };

            auto fuse_local_local=[&](size_t i,OpCode fused)
            {
                const Instruction &a=code[i];
                const Instruction &b=code[i+1];
                const Instruction &d=code[i+3];
                if(a.op==OpCode::LoadLocal && b.op==OpCode::LoadLocal
                    && d.op==OpCode::StoreLocal && a.a==d.a)
                {
                    code[i]=Instruction{fused,a.a,b.a,0};
                    code[i+1].op=OpCode::Nop;
                    code[i+2].op=OpCode::Nop;
                    code[i+3].op=OpCode::Nop;
                    return true;
                }
                return false;
            };

            auto fuse_local_cmp_local=[&](size_t i,OpCode fused)
            {
                const Instruction &a=code[i];
                const Instruction &b=code[i+1];
                const Instruction &d=code[i+3];
                if(a.op==OpCode::LoadLocal && b.op==OpCode::LoadLocal
                    && d.op==OpCode::JumpIfFalse)
                {
                    code[i]=Instruction{fused,a.a,b.a,d.a};
                    code[i+1].op=OpCode::Nop;
                    code[i+2].op=OpCode::Nop;
                    code[i+3].op=OpCode::Nop;
                    return true;
                }
                return false;
            };

            for(size_t i=0;i+3<code.size();++i)
            {
                const Instruction &a=code[i];
                const Instruction &b=code[i+1];
                const Instruction &c=code[i+2];
                const Instruction &d=code[i+3];

                if(c.op==OpCode::Add && fuse_local_const(i,OpCode::AddLocalConst)) continue;
                if(c.op==OpCode::Sub && fuse_local_const(i,OpCode::SubLocalConst)) continue;
                if(c.op==OpCode::Mul && fuse_local_const(i,OpCode::MulLocalConst)) continue;
                if(c.op==OpCode::Div && fuse_local_const(i,OpCode::DivLocalConst)) continue;
                if(c.op==OpCode::Mod && fuse_local_const(i,OpCode::ModLocalConst)) continue;
                if(c.op==OpCode::BitAnd && fuse_local_const(i,OpCode::BitAndLocalConst)) continue;
                if(c.op==OpCode::BitOr && fuse_local_const(i,OpCode::BitOrLocalConst)) continue;
                if(c.op==OpCode::BitXor && fuse_local_const(i,OpCode::BitXorLocalConst)) continue;
                if(c.op==OpCode::Shl && fuse_local_const(i,OpCode::ShlLocalConst)) continue;
                if(c.op==OpCode::Shr && fuse_local_const(i,OpCode::ShrLocalConst)) continue;

                if(c.op==OpCode::Add && fuse_local_local(i,OpCode::AddLocalLocal)) continue;
                if(c.op==OpCode::Sub && fuse_local_local(i,OpCode::SubLocalLocal)) continue;
                if(c.op==OpCode::Mul && fuse_local_local(i,OpCode::MulLocalLocal)) continue;
                if(c.op==OpCode::Div && fuse_local_local(i,OpCode::DivLocalLocal)) continue;
                if(c.op==OpCode::Mod && fuse_local_local(i,OpCode::ModLocalLocal)) continue;
                if(c.op==OpCode::BitAnd && fuse_local_local(i,OpCode::BitAndLocalLocal)) continue;
                if(c.op==OpCode::BitOr && fuse_local_local(i,OpCode::BitOrLocalLocal)) continue;
                if(c.op==OpCode::BitXor && fuse_local_local(i,OpCode::BitXorLocalLocal)) continue;
                if(c.op==OpCode::Shl && fuse_local_local(i,OpCode::ShlLocalLocal)) continue;
                if(c.op==OpCode::Shr && fuse_local_local(i,OpCode::ShrLocalLocal)) continue;

                if(c.op==OpCode::Lt && fuse_local_cmp(i,OpCode::JumpIfLocalGeConst)) continue;
                if(c.op==OpCode::Le && fuse_local_cmp(i,OpCode::JumpIfLocalGtConst)) continue;
                if(c.op==OpCode::Gt && fuse_local_cmp(i,OpCode::JumpIfLocalLeConst)) continue;
                if(c.op==OpCode::Ge && fuse_local_cmp(i,OpCode::JumpIfLocalLtConst)) continue;
                if(c.op==OpCode::Eq && fuse_local_cmp(i,OpCode::JumpIfLocalNeConst)) continue;
                if(c.op==OpCode::Ne && fuse_local_cmp(i,OpCode::JumpIfLocalEqConst)) continue;

                if(c.op==OpCode::Lt && fuse_local_cmp_local(i,OpCode::JumpIfLocalGeLocal)) continue;
                if(c.op==OpCode::Le && fuse_local_cmp_local(i,OpCode::JumpIfLocalGtLocal)) continue;
                if(c.op==OpCode::Gt && fuse_local_cmp_local(i,OpCode::JumpIfLocalLeLocal)) continue;
                if(c.op==OpCode::Ge && fuse_local_cmp_local(i,OpCode::JumpIfLocalLtLocal)) continue;
                if(c.op==OpCode::Eq && fuse_local_cmp_local(i,OpCode::JumpIfLocalNeLocal)) continue;
                if(c.op==OpCode::Ne && fuse_local_cmp_local(i,OpCode::JumpIfLocalEqLocal)) continue;
            }
        }
    }

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
            if(binary->GetOp()==TokenType::And)
            {
                if(!BuildExpr(func,binary->GetLeft()))
                    return false;
                const size_t jmp_false=Emit(func,OpCode::JumpIfFalse,-1);
                if(!BuildExpr(func,binary->GetRight()))
                    return false;
                const size_t jmp_end=Emit(func,OpCode::Jump,-1);

                const size_t false_pos=func.code.size();
                func.code[jmp_false].a=static_cast<int32_t>(false_pos);
                const int32_t false_idx=AddConst(func,AstValue::MakeBool(false));
                Emit(func,OpCode::PushConst,false_idx);

                const size_t end_pos=func.code.size();
                func.code[jmp_end].a=static_cast<int32_t>(end_pos);
                return true;
            }

            if(binary->GetOp()==TokenType::Or)
            {
                if(!BuildExpr(func,binary->GetLeft()))
                    return false;
                const size_t jmp_false=Emit(func,OpCode::JumpIfFalse,-1);
                const int32_t true_idx=AddConst(func,AstValue::MakeBool(true));
                Emit(func,OpCode::PushConst,true_idx);
                const size_t jmp_end=Emit(func,OpCode::Jump,-1);

                const size_t rhs_pos=func.code.size();
                func.code[jmp_false].a=static_cast<int32_t>(rhs_pos);
                if(!BuildExpr(func,binary->GetRight()))
                    return false;

                const size_t end_pos=func.code.size();
                func.code[jmp_end].a=static_cast<int32_t>(end_pos);
                return true;
            }

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

        if(const auto *assign=dynamic_cast<const AssignExpr *>(expr))
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
            Emit(func,OpCode::LoadLocal,local);
            return true;
        }

        if(const auto *cast=dynamic_cast<const CastExpr *>(expr))
        {
            if(!BuildExpr(func,cast->GetValue()))
                return false;
            Emit(func,OpCode::Cast,static_cast<int32_t>(cast->GetTargetType()));
            return true;
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

        if(dynamic_cast<const EnumDeclStmt *>(stmt))
            return true;

        if(const auto *var=dynamic_cast<const VarDeclStmt *>(stmt))
        {
            if(GetLocal(var->GetName())>=0)
            {
                error="duplicate local: "+var->GetName();
                return false;
            }
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

        if(const auto *brk=dynamic_cast<const BreakStmt *>(stmt))
        {
            if(loop_stack.empty())
            {
                error="break used outside loop/switch";
                return false;
            }
            const size_t jump_index=Emit(func,OpCode::Jump,-1);
            loop_stack.back().breaks.push_back(jump_index);
            return true;
        }

        if(const auto *cont=dynamic_cast<const ContinueStmt *>(stmt))
        {
            if(loop_stack.empty() || !loop_stack.back().allow_continue)
            {
                error="continue used outside loop";
                return false;
            }
            const size_t jump_index=Emit(func,OpCode::Jump,-1);
            loop_stack.back().continues.push_back(jump_index);
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

            LoopContext loop_ctx;
            loop_ctx.allow_continue=true;
            loop_ctx.continue_target=loop_start;
            loop_stack.push_back(loop_ctx);

            if(!BuildBlock(func,ws->GetBody()))
            {
                loop_stack.pop_back();
                return false;
            }
            Emit(func,OpCode::Jump,static_cast<int32_t>(loop_start));

            const size_t end_index=func.code.size();
            func.code[jmp_out].a=static_cast<int32_t>(end_index);

            LoopContext &ctx=loop_stack.back();
            for(const size_t idx:ctx.continues)
                func.code[idx].a=static_cast<int32_t>(ctx.continue_target);
            for(const size_t idx:ctx.breaks)
                func.code[idx].a=static_cast<int32_t>(end_index);
            loop_stack.pop_back();
            return true;
        }

        if(const auto *dw=dynamic_cast<const DoWhileStmt *>(stmt))
        {
            const size_t loop_start=func.code.size();

            LoopContext loop_ctx;
            loop_ctx.allow_continue=true;
            loop_ctx.continue_target=loop_start;
            loop_stack.push_back(loop_ctx);

            if(!BuildBlock(func,dw->GetBody()))
            {
                loop_stack.pop_back();
                return false;
            }

            const size_t cond_pos=func.code.size();
            loop_stack.back().continue_target=cond_pos;

            if(!BuildExpr(func,dw->GetCond()))
            {
                loop_stack.pop_back();
                return false;
            }

            const size_t jmp_out=Emit(func,OpCode::JumpIfFalse,-1);
            Emit(func,OpCode::Jump,static_cast<int32_t>(loop_start));

            const size_t end_index=func.code.size();
            func.code[jmp_out].a=static_cast<int32_t>(end_index);

            LoopContext &ctx=loop_stack.back();
            for(const size_t idx:ctx.continues)
                func.code[idx].a=static_cast<int32_t>(ctx.continue_target);
            for(const size_t idx:ctx.breaks)
                func.code[idx].a=static_cast<int32_t>(end_index);
            loop_stack.pop_back();
            return true;
        }

        if(const auto *fs=dynamic_cast<const ForStmt *>(stmt))
        {
            if(const Stmt *init=fs->GetInit())
            {
                if(!BuildStmt(func,init))
                    return false;
            }

            const size_t loop_start=func.code.size();
            size_t jmp_out=static_cast<size_t>(-1);
            if(const Expr *cond=fs->GetCond())
            {
                if(!BuildExpr(func,cond))
                    return false;
                jmp_out=Emit(func,OpCode::JumpIfFalse,-1);
            }

            LoopContext loop_ctx;
            loop_ctx.allow_continue=true;
            loop_ctx.continue_target=loop_start;
            loop_stack.push_back(loop_ctx);

            if(!BuildBlock(func,fs->GetBody()))
            {
                loop_stack.pop_back();
                return false;
            }

            const size_t post_start=func.code.size();
            loop_stack.back().continue_target=post_start;

            if(const Expr *post=fs->GetPost())
            {
                if(!BuildExpr(func,post))
                {
                    loop_stack.pop_back();
                    return false;
                }
                Emit(func,OpCode::Pop);
            }

            Emit(func,OpCode::Jump,static_cast<int32_t>(loop_start));

            const size_t end_index=func.code.size();
            if(jmp_out!=static_cast<size_t>(-1))
                func.code[jmp_out].a=static_cast<int32_t>(end_index);

            LoopContext &ctx=loop_stack.back();
            for(const size_t idx:ctx.continues)
                func.code[idx].a=static_cast<int32_t>(ctx.continue_target);
            for(const size_t idx:ctx.breaks)
                func.code[idx].a=static_cast<int32_t>(end_index);
            loop_stack.pop_back();
            return true;
        }

        if(const auto *sw=dynamic_cast<const SwitchStmt *>(stmt))
        {
            if(!BuildExpr(func,sw->GetExpr()))
                return false;

            const std::string temp_name="$switch_tmp_"+std::to_string(locals.size());
            const int32_t temp_local=AddLocal(temp_name);
            Emit(func,OpCode::StoreLocal,temp_local);

            size_t pending_not=static_cast<size_t>(-1);
            std::vector<size_t> case_jumps;
            const auto &cases=sw->GetCases();
            for(const auto &case_item:cases)
            {
                const size_t compare_pos=func.code.size();
                if(pending_not!=static_cast<size_t>(-1))
                    func.code[pending_not].a=static_cast<int32_t>(compare_pos);

                Emit(func,OpCode::LoadLocal,temp_local);
                if(!BuildExpr(func,case_item.expr.get()))
                    return false;
                Emit(func,OpCode::Eq);
                pending_not=Emit(func,OpCode::JumpIfFalse,-1);
                const size_t jmp_case=Emit(func,OpCode::Jump,-1);
                case_jumps.push_back(jmp_case);
            }

            const size_t after_compares=func.code.size();
            if(pending_not!=static_cast<size_t>(-1))
                func.code[pending_not].a=static_cast<int32_t>(after_compares);

            const size_t default_jump=Emit(func,OpCode::Jump,-1);

            LoopContext loop_ctx;
            loop_ctx.allow_continue=false;
            loop_stack.push_back(loop_ctx);

            for(size_t i=0;i<cases.size();++i)
            {
                const size_t case_pos=func.code.size();
                func.code[case_jumps[i]].a=static_cast<int32_t>(case_pos);
                if(!BuildBlock(func,cases[i].block.get()))
                {
                    loop_stack.pop_back();
                    return false;
                }

                if(switch_auto_break)
                {
                    const auto &stmts=cases[i].block->GetStatements();
                    const Stmt *last=stmts.empty()?nullptr:stmts.back().get();
                    if(!dynamic_cast<const BreakStmt *>(last)
                        && !dynamic_cast<const ReturnStmt *>(last)
                        && !dynamic_cast<const GotoStmt *>(last))
                    {
                        const size_t auto_break=Emit(func,OpCode::Jump,-1);
                        loop_stack.back().breaks.push_back(auto_break);
                    }
                }
            }

            const size_t default_pos=func.code.size();
            if(const BlockStmt *def_block=sw->GetDefault())
            {
                func.code[default_jump].a=static_cast<int32_t>(default_pos);
                if(!BuildBlock(func,def_block))
                {
                    loop_stack.pop_back();
                    return false;
                }
            }
            else
            {
                func.code[default_jump].a=static_cast<int32_t>(default_pos);
            }

            const size_t end_index=func.code.size();
            LoopContext &ctx=loop_stack.back();
            for(const size_t idx:ctx.breaks)
                func.code[idx].a=static_cast<int32_t>(end_index);
            loop_stack.pop_back();
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

        const auto &params=func->GetParams();
        for(const auto &param:params)
        {
            if(GetLocal(param.name)>=0)
            {
                error="duplicate param name: "+param.name;
                return false;
            }
            AddLocal(param.name);
        }

        if(!BuildBlock(out_func,func->GetBody()))
            return false;

        PeepholeFused(out_func);

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
                std::string label_list;
                label_list.reserve(labels.size()*8);
                for(const auto &kv:labels)
                {
                    if(!label_list.empty())
                        label_list.append(",");
                    label_list.append(kv.first);
                }

                error="bytecode label not found: "+p.label+" (known: "+label_list+")";
                return false;
            }
            out_func.code[p.index].a=static_cast<int32_t>(it->second);
        }

        out_func.local_count=static_cast<int32_t>(locals.size());
        out_func.param_count=static_cast<int32_t>(params.size());
        out_func.fast_int=ComputeFastInt(out_func);
        out_func.fast_float=ComputeFastFloat(out_func);
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
