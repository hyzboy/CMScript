#include <hgl/devil/DevilContext.h>
#include <hgl/devil/DevilModule.h>
#include "DevilFunc.h"

namespace hgl::devil
{
    AstValue Context::ExecuteFunction(Func *func,const char *start_label)
    {
        if(!func || !module)
        {
            LogError("%s","ExecuteFunction missing func/module");
            return AstValue::MakeVoid();
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
                    LogError("%s",("label not found: "+res.label).c_str());
                    State=dvsStop;
                    current_index=-1;
                    return AstValue::MakeVoid();
                }
                index=it->second;
                continue;
            }

            LogError("%s",res.error.c_str());
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

