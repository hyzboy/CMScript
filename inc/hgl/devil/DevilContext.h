#pragma once

#include <stdarg.h>
#include <cstdint>
#include <string>
#include <vector>
#include <hgl/log/Log.h>
#include <hgl/devil/DevilValue.h>

namespace hgl::devil
{
    class Module;
    class Func;

    /**
    * 虚拟机状态
    */
    enum VMState
    {
        dvsRun,     //运行
        dvsPause,   //暂停
        dvsStop,    //停止
    };//enum VMState

    /**
     * 虚拟机执行控制上下文
     */
    class Context
    {
        OBJECT_LOGGER

        Module *module=nullptr;
        std::string current_func;
        int current_index=-1;

    protected:

        VMState State=dvsStop;                                               ///<虚拟机状态

    public:

        explicit Context(Module *dm=nullptr)
            : module(dm)
        {
        }

        virtual ~Context()=default;

        void SetModule(Module *dm)
        {
            module=dm;
        }

        AstValue ExecuteFunction(Func *func,const char *start_label);

        virtual bool Start(Func *,...);
        virtual bool Start(const char *);
        virtual bool Start(const char *,const char *);                        ///<开始运行虚拟机
        virtual bool StartFlag(Func *,const char *);
        virtual bool StartFlag(const char *,const char *);
        virtual bool Run(const char *func_name=nullptr);                      ///<运行虚拟机，如Start或End状态则从开始运行，Pause状态会继续运行
        virtual void Pause();                                                ///<暂停虚拟机，仅能从Run状态变为Pause，其它情况会失败
        virtual void Stop();                                                 ///<终止虚拟机，从任何状况变为Start状态

        virtual bool Goto(const char *);                                     ///<跳转到指定位置
        virtual bool Goto(const char *,const char *);                        ///<跳转到指定位置

        virtual bool GetCurrentState(std::string &,int &);                   ///<取得当前状态

        virtual bool SaveState(std::vector<uint8_t> &);                      ///<保存状态(字节)
        virtual bool LoadState(const std::vector<uint8_t> &);                ///<加载状态(字节)
    };//class Context
}//namespace hgl::devil
