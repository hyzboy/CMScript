#include <hgl/devil/DevilContext.h>
#include <hgl/devil/DevilModule.h>
#include "DevilFunc.h"

namespace hgl::devil
{
    namespace
    {
        AstValue CastToType(const AstValue &value,TokenType type)
        {
            switch(type)
            {
                case TokenType::Bool:   return AstValue::MakeBool(value.ToBool());
                case TokenType::Int:
                case TokenType::Int8:
                case TokenType::Int16:  return AstValue::MakeInt(value.ToInt());
                case TokenType::UInt:
                case TokenType::UInt8:
                case TokenType::UInt16: return AstValue::MakeUInt(value.ToUInt());
                case TokenType::Float:  return AstValue::MakeFloat(value.ToFloat());
                case TokenType::String: return AstValue::MakeString(value.ToString());
                default:                return AstValue::MakeVoid();
            }
        }
    }

    AstValue Context::ExecuteFunction(Func *func,const char *start_label)
    {
        const std::vector<AstValue> empty_args;
        return ExecuteFunction(func,start_label,empty_args);
    }

    AstValue Context::ExecuteFunction(Func *func,const char *start_label,const std::vector<AstValue> &args)
    {
        if(!func || !module)
        {
            LogError("%s","ExecuteFunction missing func/module");
            return AstValue::MakeVoid();
        }

        const auto &params=func->GetParams();
        if(args.size()!=params.size())
        {
            LogError("%s",("param count mismatch: func="+func->func_name
                +" expected="+std::to_string(params.size())
                +" got="+std::to_string(args.size())).c_str());
            return AstValue::MakeVoid();
        }

        if(use_bytecode && module->IsBytecodeEnabled() && (!start_label || !*start_label))
        {
            BytecodeModule *bc_module=module->GetBytecodeModule();
            if(bc_module)
            {
                bytecode_vm.SetModule(bc_module);
                current_func=func->func_name;
                current_index=-1;
                State=dvsRun;

                if(bytecode_vm.Execute(func->func_name,args))
                {
                    State=dvsStop;
                    return bytecode_vm.GetLastResult();
                }

                LogError("%s",("bytecode execute failed: "+bytecode_vm.GetError()).c_str());
                State=dvsStop;
            }
        }

        const BlockStmt *body=func->GetBody();
        if(!body)
        {
            LogError("%s","ExecuteFunction missing AST body");
            return AstValue::MakeVoid();
        }

        ExecContext ctx;
        ctx.module=module;
        ctx.context=this;
        ctx.func=func;
        for(size_t i=0;i<params.size();++i)
            ctx.locals.emplace(params[i].name,CastToType(args[i],params[i].type));

        size_t index=0;
        if(start_label && *start_label)
        {
            const auto &labels=func->GetLabels();
            const auto it=labels.find(start_label);
            if(it==labels.end())
            {
                LogError("%s",("label not found: "+std::string(start_label)).c_str());
                return AstValue::MakeVoid();
            }
            index=it->second;
        }

        current_func=func->func_name;
        current_index=static_cast<int>(index);
        State=dvsRun;

        const auto &stmts=body->GetStatements();
        while(index<stmts.size())
        {
            current_index=static_cast<int>(index);
            ExecResult res=stmts[index]->Exec(ctx);

            if(res.flow==ExecFlow::Normal)
            {
                index++;
                continue;
            }

            if(res.flow==ExecFlow::Return)
            {
                State=dvsStop;
                current_index=-1;
                return res.value;
            }

            if(res.flow==ExecFlow::Goto)
            {
                const auto &labels=func->GetLabels();
                const auto it=labels.find(res.label);
                if(it==labels.end())
                {
                    LogError("%s",("label not found: "+res.label+" func="+current_func).c_str());
                    State=dvsStop;
                    current_index=-1;
                    return AstValue::MakeVoid();
                }
                index=it->second;
                continue;
            }

            if(res.flow==ExecFlow::Break || res.flow==ExecFlow::Continue)
            {
                LogError("%s",("exec failed: func="+current_func
                    +" index="+std::to_string(current_index)
                    +(ctx.current_loc.line>0?" line="+std::to_string(ctx.current_loc.line):std::string())
                    +(ctx.current_loc.column>0?" column="+std::to_string(ctx.current_loc.column):std::string())
                    +(!ctx.current_loc.line_text.empty()?"\n"+ctx.current_loc.line_text+"\n"+ctx.current_loc.caret_line:std::string())
                    +" error=break/continue used outside loop").c_str());
                State=dvsStop;
                current_index=-1;
                return AstValue::MakeVoid();
            }

            LogError("%s",("exec failed: func="+current_func
                +" index="+std::to_string(current_index)
                +(ctx.current_loc.line>0?" line="+std::to_string(ctx.current_loc.line):std::string())
                +(ctx.current_loc.column>0?" column="+std::to_string(ctx.current_loc.column):std::string())
                +(!ctx.current_loc.line_text.empty()?"\n"+ctx.current_loc.line_text+"\n"+ctx.current_loc.caret_line:std::string())
                +" error="+res.error).c_str());
            State=dvsStop;
            current_index=-1;
            return AstValue::MakeVoid();
        }

        State=dvsStop;
        current_index=-1;
        return AstValue::MakeVoid();
    }

    bool Context::Start(Func *func,...)
    {
        ExecuteFunction(func,nullptr);
        return true;
    }

    bool Context::Start(const char *func_name)
    {
        if(!module)
            return false;
        Func *func=module->GetScriptFunc(func_name);
        if(!func)
            return false;
        ExecuteFunction(func,nullptr);
        return true;
    }

    bool Context::StartFlag(Func *func,const char *goto_flag)
    {
        ExecuteFunction(func,goto_flag);
        return true;
    }

    bool Context::StartFlag(const char *func_name,const char *goto_flag)
    {
        if(!module)
            return false;
        Func *func=module->GetScriptFunc(func_name);
        if(!func)
            return false;
        ExecuteFunction(func,goto_flag);
        return true;
    }

    bool Context::Start(const char *func_name,const char *goto_flag)
    {
        return StartFlag(func_name,goto_flag);
    }

    bool Context::Run(const char *func_name)
    {
        if(func_name)
            return Start(func_name);
        if(!current_func.empty())
            return Start(current_func.c_str());
        return false;
    }

    void Context::Pause()
    {
        State=dvsPause;
    }

    void Context::Stop()
    {
        State=dvsStop;
    }

    bool Context::Goto(const char *)
    {
        LogError("%s","Goto is not supported during AST execution");
        return false;
    }

    bool Context::Goto(const char *,const char *)
    {
        LogError("%s","Goto is not supported during AST execution");
        return false;
    }

    bool Context::GetCurrentState(std::string &func_name,int &func_line)
    {
        func_name=current_func;
        func_line=current_index;
        return !func_name.empty();
    }

    bool Context::SaveState(std::vector<uint8_t> &)
    {
        LogError("%s","SaveState is not supported during AST execution");
        return false;
    }

    bool Context::LoadState(const std::vector<uint8_t> &)
    {
        LogError("%s","LoadState is not supported during AST execution");
        return false;
    }
}

