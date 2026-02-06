#include"DevilContext.h"
#include"DevilModule.h"

namespace hgl
{
    void DevilScriptContext::ClearStack()
    {
        run_state.Clear();
    }

    bool DevilScriptContext::RunContext()
    {
        while(true)
        {
            while(cur_state->index<
                  cur_state->func->command.GetCount())
            {
                ScriptFuncRunState *sfrs=cur_state;                     //cmd->run有可能更改cur_state，所以这里保存，以保证sfrs->index++正确

                DevilCommand *cmd=sfrs->func->command[sfrs->index++];   //cmd->run有可能更改index,所以这里先加

                #ifdef _DEBUG
                LogInfo(U16_TEXT("%s"),
                    (U16_TEXT("run to func: \"")+sfrs->func->func_name+U16_TEXT("\" line: ")
                     +U16String::numberOf(sfrs->index-1)).c_str());
                #endif//

                if(cmd->Run())                  //有可能更改cur_state和index
                {
                    if(State==dvsStop)          //退出
                        return(true);
                    else
                    if(State==dvsPause)         //暂停
                        return(true);
                    else
                    if(State==dvsRun)
                        continue;
                }
                else
                {
                    LogError(U16_TEXT("%s"),
                             (U16_TEXT("run error,func: ")+sfrs->func->func_name+U16_TEXT(",code index: ")
                              +U16String::numberOf(sfrs->index-1)).c_str());
                    return(false);
                }

                if(cur_state)
                    break;      //到这里不正常
            }

            //当前函数运行完毕
            if(!Return())       //返回上一级调用函数失败表示运行结束了
                return(true);
        }
    }

    void DevilScriptContext::ScriptFuncCall(DevilFunc *func)
    {
        ScriptFuncRunState state;
        state.func=func;
        state.index=0;

        run_state.Add(state);
        cur_state=&run_state[run_state.GetCount()-1];
    }

    bool DevilScriptContext::Goto(DevilFunc *func,int index)
    {
        if(index<0)
        {
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("DevilEngine执行GOTO时，指令索引不正确.funcname: ")
                      +func->func_name+U16_TEXT("code index: ")+U16String::numberOf(index)).c_str());
            return(false);
        }

        if(cur_state->func==func)
        {
            cur_state->index=index;     //跳转
            return(true);
        }
        else
        {
            LogError(U16_TEXT("%s"),
                     (U16String(U16_TEXT("DevilEngine执行GOTO时，当前函数对应不正确!\n"))
                      +U16_TEXT("要求为:")+func->func_name
                      +U16_TEXT("实质为:")+cur_state->func->func_name).c_str());

            return(false);
        }
    }

    bool DevilScriptContext::Goto(const u16char *func_name,const u16char *flag)
    {
        ClearStack();

        DevilFunc *func;

        func=module->GetScriptFunc(func_name);

        if(func)
        {
            ScriptFuncCall(func);
        }
        else
        {
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("没有找到起始函数: ")+U16String(func_name)).c_str());
            return(false);
        }

        State=dvsRun;

        return Goto(flag);
    }

    bool DevilScriptContext::Return()
    {
        run_state.Delete(run_state.GetCount()-1);       //删除最后一个，即当前函数

        if(run_state.GetCount())                            //检查堆栈中还有没有数据
        {
            cur_state=&run_state[run_state.GetCount()-1];   //退到上一级函数

            return(true);
        }
        else
            return(false);
    }

    bool Start(DevilFunc *,const va_list &);

    bool DevilScriptContext::Start(const u16char *func_name)
    {
        ClearStack();

        DevilFunc *func;

        func=module->GetScriptFunc(func_name);

        if(func)
        {
            ScriptFuncCall(func);
        }
        else
        {
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("没有找到起始函数: ")+U16String(func_name)).c_str());
            return(false);
        }

        State=dvsRun;

        return RunContext();
    }

    bool DevilScriptContext::StartFlag(const u16char *func_name,const u16char *goto_flag)
    {
        DevilFunc *func;

        func=module->GetScriptFunc(func_name);

        if(!func)return(false);

        if(Goto(func_name,goto_flag))
            return RunContext();
        else
            return(false);
    }

    bool DevilScriptContext::Run(const u16char *func_name)
    {
        if(run_state.GetCount()>0)
        {
            cur_state=&run_state[run_state.GetCount()-1];   //取最后一个

            if(cur_state->func)
            {
                if(func_name)
                {
                    if(cur_state->func->func_name.Comp(func_name)!=0)
                        return Start(func_name);
                }
            }
            else
            {
                LogError(U16_TEXT("%s"),U16_TEXT("当前堆栈中的函数指针为NULL"));
                return(false);
            }
        }
        else
        {
            if(!func_name)  //未指定从那个函数开始
            {
                LogError(U16_TEXT("%s"),U16_TEXT("不知道从那里开始运行"));
                return(false);
            }
            else
                return Start(func_name);
        }

        State=dvsRun;

        return RunContext();
    }

    void DevilScriptContext::Pause()
    {
        State=dvsPause;
    }

    void DevilScriptContext::Stop()
    {
        State=dvsStop;
        ClearStack();

        cur_state=nullptr;
    }

    bool DevilScriptContext::Goto(const u16char *flag)
    {
        if(!cur_state)
        {
//          PutError(u"跳转时，虚拟机的当前运行函数不存在！");
//          return(false);

            if(run_state.GetCount()>0)
                cur_state=&run_state[run_state.GetCount()-1];   //取最后一个
            else
            {
                LogError(U16_TEXT("%s"),U16_TEXT("跳转时，虚拟机的当前运行函数不存在！呼叫堆栈中也没有函数！"));

                return(false);
            }
        }

        int index;

        if(!cur_state->func->goto_flag.Get(flag,index))
        {
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("没有在函数")+cur_state->func->func_name+U16_TEXT("内找到跳转标识")
                      +U16String(flag)).c_str());
            return(false);
        }

        return Goto(cur_state->func,index);
    }

    bool DevilScriptContext::GetCurrentState(U16String &func_name,int &func_line)
    {
        if(!cur_state)return(false);

        func_name=cur_state->func->func_name;
        func_line=cur_state->index;

        return(true);
    }

    bool DevilScriptContext::SaveState(io::DataOutputStream *p)
    {
        if(!p->WriteInt32(run_state.GetCount()))return(false);

        for(int i=0;i<run_state.GetCount();i++)
        {
            if(!p->WriteUTF16LEString(run_state[i].func->func_name))return(false);
            if(!p->WriteInt32(run_state[i].index))return(false);
        }

        return(true);
    }

    bool DevilScriptContext::LoadState(io::DataInputStream *p)
    {
        ClearStack();
        cur_state=nullptr;

        int number;
        U16String name;
        int index;

        if(!p->ReadInt32(number))return(false);

        for(int i=0;i<number;i++)
        {
            if(!p->ReadUTF16LEString(name))return(false);
            if(!p->ReadInt32(index))return(false);

            ScriptFuncRunState sfrs;
            sfrs.func=module->GetScriptFunc(name);
            sfrs.index=index;

            run_state.Add(sfrs);
        }

        if(run_state.GetCount()>0)
            cur_state=&run_state[run_state.GetCount()-1];

        return(true);
    }
}//namespace hgl
