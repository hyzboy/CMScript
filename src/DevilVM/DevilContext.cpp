#include <hgl/devil/DevilContext.h>
#include <hgl/devil/DevilModule.h>
#include <hgl/type/StdByteBuffer.h>
#include"DevilCommand.h"
#include"DevilFunc.h"
#include <cstdint>
#include <cstring>

namespace hgl::devil
{
    void Context::ClearStack()
    {
        run_state.clear();
    }

    bool Context::RunContext()
    {
        while(true)
        {
            while(cur_state->index<
                                    static_cast<int>(cur_state->func->command.size()))
            {
                ScriptFuncRunState *sfrs=cur_state;                     //cmd->run有可能更改cur_state，所以这里保存，以保证sfrs->index++正确

                                Command *cmd=sfrs->func->command[sfrs->index++].get();   //cmd->run有可能更改index,所以这里先加

                #ifdef _DEBUG
                LogInfo("%s",
                    ("run to func: \""+sfrs->func->func_name+"\" line: "
                     +std::to_string(sfrs->index-1)).c_str());
                #endif//

                if(cmd->Run(this))                  //有可能更改cur_state和index
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
                    LogError("%s",
                             ("run error,func: "+sfrs->func->func_name+",code index: "
                              +std::to_string(sfrs->index-1)).c_str());
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

    void Context::ScriptFuncCall(Func *func)
    {
        ScriptFuncRunState state;
        state.func=func;
        state.index=0;

        run_state.push_back(state);
        cur_state=&run_state[run_state.size()-1];
    }

    bool Context::Goto(Func *func,int index)
    {
        if(index<0)
        {
            LogError("%s",
                     ("DevilEngine执行GOTO时，指令索引不正确.funcname: "
                      +func->func_name+"code index: "+std::to_string(index)).c_str());
            return(false);
        }

        if(cur_state->func==func)
        {
            cur_state->index=index;     //跳转
            return(true);
        }
        else
        {
            LogError("%s",
                     (std::string("DevilEngine执行GOTO时，当前函数对应不正确!\n")
                      +"要求为:"+func->func_name
                      +"实质为:"+cur_state->func->func_name).c_str());

            return(false);
        }
    }

    bool Context::Goto(const char *func_name,const char *flag)
    {
        ClearStack();

        Func *func;

        func=module->GetScriptFunc(func_name);

        if(func)
        {
            ScriptFuncCall(func);
        }
        else
        {
            LogError("%s",
                     ("没有找到起始函数: "+std::string(func_name)).c_str());
            return(false);
        }

        State=dvsRun;

        return Goto(flag);
    }

    bool Context::Return()
    {
        if(run_state.empty())
            return(false);

        run_state.pop_back();                                                                //删除最后一个，即当前函数

        if(!run_state.empty())                              //检查堆栈中还有没有数据
        {
            cur_state=&run_state[run_state.size()-1];       //退到上一级函数

            return(true);
        }
        else
            return(false);
    }

    bool Start(Func *,const va_list &);

    bool Context::Start(const char *func_name)
    {
        ClearStack();

        Func *func;

        func=module->GetScriptFunc(func_name);

        if(func)
        {
            ScriptFuncCall(func);
        }
        else
        {
            LogError("%s",
                     ("没有找到起始函数: "+std::string(func_name)).c_str());
            return(false);
        }

        State=dvsRun;

        return RunContext();
    }

    bool Context::Start(Func *func,...)
    {
        if(!func)
            return(false);

        ClearStack();
        ScriptFuncCall(func);
        State=dvsRun;

        return RunContext();
    }

    bool Context::StartFlag(const char *func_name,const char *goto_flag)
    {
        Func *func;

        func=module->GetScriptFunc(func_name);

        if(!func)return(false);

        if(Goto(func_name,goto_flag))
            return RunContext();
        else
            return(false);
    }

    bool Context::StartFlag(Func *func,const char *goto_flag)
    {
        if(!func)
            return(false);

        ClearStack();
        ScriptFuncCall(func);
        State=dvsRun;

        if(Goto(goto_flag))
            return RunContext();
        else
            return(false);
    }

    bool Context::Start(const char *func_name,const char *goto_flag)
    {
        return StartFlag(func_name,goto_flag);
    }

    bool Context::Run(const char *func_name)
    {
        if(!run_state.empty())
        {
            cur_state=&run_state[run_state.size()-1];       //取最后一个

            if(cur_state->func)
            {
                if(func_name)
                {
                    if(cur_state->func->func_name!=func_name)
                        return Start(func_name);
                }
            }
            else
            {
                LogError("%s","当前堆栈中的函数指针为NULL");
                return(false);
            }
        }
        else
        {
            if(!func_name)  //未指定从那个函数开始
            {
                LogError("%s","不知道从那里开始运行");
                return(false);
            }
            else
                return Start(func_name);
        }

        State=dvsRun;

        return RunContext();
    }

    void Context::Pause()
    {
        State=dvsPause;
    }

    void Context::Stop()
    {
        State=dvsStop;
        ClearStack();

        cur_state=nullptr;
    }

    bool Context::Goto(const char *flag)
    {
        if(!cur_state)
        {
//          PutError(u"跳转时，虚拟机的当前运行函数不存在！");
//          return(false);

            if(!run_state.empty())
                cur_state=&run_state[run_state.size()-1];       //取最后一个
            else
            {
                LogError("%s","跳转时，虚拟机的当前运行函数不存在！呼叫堆栈中也没有函数！");

                return(false);
            }
        }

        int index;

        const auto it=cur_state->func->goto_flag.find(flag);
        if(it==cur_state->func->goto_flag.end())
        {
            LogError("%s",
                     ("没有在函数"+cur_state->func->func_name+"内找到跳转标识"
                      +std::string(flag)).c_str());
            return(false);
        }

        index=it->second;

        return Goto(cur_state->func,index);
    }

    bool Context::GetCurrentState(std::string &func_name,int &func_line)
    {
        if(!cur_state)return(false);

        func_name=cur_state->func->func_name;
        func_line=cur_state->index;

        return(true);
    }

    bool Context::SaveState(std::vector<uint8_t> &out_bytes)
    {
        ByteWriter writer(out_bytes);
        writer.reset();
        writer.u8(static_cast<uint8_t>(run_state.size()));

        for(const auto &state : run_state)
        {
            if(!state.func)
                return(false);

            if(writer.string(state.func->func_name) < 0)
                return(false);

            writer.i32(static_cast<int32_t>(state.index));
        }

        return(true);
    }

    bool Context::LoadState(const std::vector<uint8_t> &in_bytes)
    {
        ClearStack();
        cur_state=nullptr;

        ByteReader reader(in_bytes);

        uint8_t count=0;
        if(!reader.u8(count))
            return(false);

        for(uint8_t i=0;i<count;i++)
        {
            std::string name;
            if(reader.string(name) < 0)
                return(false);

            int32_t index=0;
            if(!reader.i32(index))
                return(false);

            ScriptFuncRunState sfrs;
            sfrs.func=module->GetScriptFunc(name);
            if(!sfrs.func)
                return(false);
            sfrs.index=index;

            run_state.push_back(sfrs);
        }

        if(!run_state.empty())
            cur_state=&run_state[run_state.size()-1];

        return(true);
    }
}//namespace hgl::devil
