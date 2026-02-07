#pragma once

#include <stdarg.h>

#include <hgl/devil/VM.h>
#include <hgl/log/Log.h>
#include <hgl/platform/compiler/EventFunc.h>
#include <hgl/type/String.h>
#include <hgl/type/StringList.h>
#include <hgl/type/UnorderedMap.h>
#include <vector>

namespace hgl
{
    namespace io
    {
        class DataInputStream;
        class DataOutputStream;
    }//namespace io

namespace devil
{
    class Func;
    class Enum;
    class ValueInterface;
    class ScriptFuncCall;
    class Goto;
    class CompGoto;
    class Return;
    struct PropertyMap;
    struct FuncMap;

    /**
    * 虚拟机状态
    */
    enum VMState
    {
        dvsRun,     //运行
        dvsPause,   //暂停
        dvsStop,    //停止
    };//enum VMState

    struct ScriptFuncRunState
    {
        Func *func;         //函数指针

        int index;          //运行到的指令编号

        bool operator==(const ScriptFuncRunState &other) const
        {
            return func == other.func && index == other.index;
        }
    };//struct ScriptFuncRunState

    /**
     * 虚拟机处理模块
     */
    class Module
    {
        OBJECT_LOGGER

        UnorderedMap<U16String,PropertyMap *>   prop_map;       //属性映射表
        UnorderedMap<U16String,FuncMap *>       func_map;       //函数映射表
        UnorderedMap<U16String,Func *>          script_func;    //脚本函数表
        UnorderedMap<U16String,Enum *>          enum_map;       //枚举映射表

    private:

        bool _MapFunc(const u16char *,void *,void *);

    public: //事件

        DefEvent(bool,OnTrueFuncCall,(const u16char *));                        ///<真实函数呼叫

    public:

        U16StringList string_list;                                    //字符串列表

    public:

        Module(){OnTrueFuncCall=nullptr;}
        virtual ~Module()=default;

        Func *GetScriptFunc(const U16String &);
        FuncMap *GetFuncMap(const U16String &);
        PropertyMap *GetPropertyMap(const U16String &);

        virtual bool MapProperty(const u16char *,void *);                      ///<映射属性(真实变量的映射，在整个模块中全局有效)
        virtual bool MapFunc(const u16char *,void *);                          ///<映射C函数
//      virtual bool MapFunc(void *,const u16char *,void *);                   ///<映射C函数,并传入一个对像
        virtual bool MapFunc(const u16char *,void *,void *);                   ///<映射C++成员函数

        virtual bool AddScript(const u16char *,int=-1);                        ///<添加脚本并编译

        virtual bool AddEnum(const u16char *,Enum *);

        virtual void Clear();                                                  ///<清除所有模块和映射

    public: //调试用函数

        #ifdef _DEBUG

        virtual void LogPropertyList();                                        ///<输出属性变量列表
        virtual void LogMapFuncList();                                         ///<输出映射函数列表
        virtual void LogScriptFuncList();                                      ///<输出脚本函数列表

        #endif//_DEBUG
    };//class Module

    /**
     * 虚拟机执行控制上下文
     */
    class Context
    {
        OBJECT_LOGGER

        Module *module;

        friend class ScriptFuncCall;
        friend class Goto;
        friend class CompGoto;
        friend class Return;

    private:

        std::vector<ScriptFuncRunState>                 run_state;  //运行状态
        ScriptFuncRunState *                            cur_state;  //当前状态
        void ClearStack();                                          //清空运行堆栈
        bool RunContext();                                          //运行

        bool Start(Func *,const va_list &);

    private:    //内部方法

        void ScriptFuncCall(Func *);
        bool Goto(Func *,int);
        bool Goto(Func *);
        bool Return();

    protected:

        VMState State;                                              ///<虚拟机状态

    public:

        explicit Context(Module *dm=nullptr)
            : module(dm), cur_state(nullptr), State(dvsStop)
        {
        }

        virtual ~Context()=default;

        void SetModule(Module *dm)
        {
            module=dm;
        }

        virtual bool Start(Func *,...);
        virtual bool Start(const u16char *);
        virtual bool Start(const u16char *,const u16char *);                 ///<开始运行虚拟机
        virtual bool StartFlag(Func *,const u16char *);
        virtual bool StartFlag(const u16char *,const u16char *);
        virtual bool Run(const u16char *func_name=0);                        ///<运行虚拟机，如Start或End状态则从开始运行，Pause状态会继续运行
        virtual void Pause();                                                ///<暂停虚拟机，仅能从Run状态变为Pause，其它情况会失败
        virtual void Stop();                                                 ///<终止虚拟机，从任何状况变为Start状态

        virtual bool Goto(const u16char *);                                  ///<跳转到指定位置
        virtual bool Goto(const u16char *,const u16char *);                  ///<跳转到指定位置

        virtual bool GetCurrentState(U16String &,int &);                     ///<取得当前状态

        virtual bool SaveState(io::DataOutputStream *);                      ///<保存状态
        virtual bool LoadState(io::DataInputStream *);                       ///<加载状态
    };//class Context

    const int ScriptMinLength=sizeof(u"func main(){}");                ///<《魔鬼》最小脚本长度
}//namespace devil
}//namespace hgl
