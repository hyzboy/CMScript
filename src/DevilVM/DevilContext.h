#pragma once

#include<hgl/script/DevilVM.h>
#include"DevilFunc.h"
#include<hgl/log/Log.h>
namespace hgl
{
    class DevilScriptModule;

    struct ScriptFuncRunState
    {
        DevilFunc *func;    //函数指针

        int index;          //运行到的指令编号

        bool operator==(const ScriptFuncRunState &other) const
        {
            return func == other.func && index == other.index;
        }
    };//struct ScriptFuncRunState

    class DevilScriptContext:public DevilContext
    {
        OBJECT_LOGGER

        DevilScriptModule *module;

        friend class DevilScriptFuncCall;
        friend class DevilGoto;
        friend class DevilCompGoto;
        friend class DevilReturn;

    private:

        ValueArray<ScriptFuncRunState>                  run_state;  //运行状态
        ScriptFuncRunState *                            cur_state;  //当前状态
        void ClearStack();                                          //清空运行堆栈
        bool RunContext();                                          //运行

        bool Start(DevilFunc *,const va_list &);

    private:    //内部方法

        void ScriptFuncCall(DevilFunc *);
        bool Goto(DevilFunc *,int);
        bool Goto(DevilFunc *);
        bool Return();

    public:

        bool Start(DevilFunc *,...);
        bool Start(const u16char *);
        bool Start(const u16char *,const u16char *);
        bool StartFlag(DevilFunc *,const u16char *);
        bool StartFlag(const u16char *,const u16char *);
        explicit DevilScriptContext(DevilScriptModule *dm=nullptr)
            : module(dm), cur_state(nullptr)
        {
        }

        void SetModule(DevilScriptModule *dm)
        {
            module=dm;
        }

        bool Run(const u16char *);
        void Pause();
        void Stop();

        bool Goto(const u16char *);
        bool Goto(const u16char *,const u16char *);

        bool GetCurrentState(U16String &,int &);

        bool SaveState(io::DataOutputStream *);
        bool LoadState(io::DataInputStream *);
    };//class DevilContext
}//namespace hgl
