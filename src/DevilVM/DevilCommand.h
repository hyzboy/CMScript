#pragma once

#include<hgl/type/String.h>
#include<hgl/type/ValueArray.h>
#include"as_tokenizer.h"
#include<hgl/log/Log.h>

using namespace angle_script;
namespace hgl
{
namespace devil
{
    class Module;
    class Context;
    class Func;
    class ValueInterface;
    template<typename T> class ValueProperty;
    template<typename T> class ScriptValue;

    union SystemFuncParam          //函数参数
    {
        void *  void_pointer;

        char    c;
        char *  str;
        u16char wc;
        u16char *wide_str;
        int     i;
        int *   int_pointer;
        uint    u;
        uint *  uint_pointer;
        float   f;
        float * float_pointer;

    #if HGL_CPU == HGL_CPU_X86_64
        double  d;
        double *double_pointer;
        uint64  ui64;
        int64   si64;
        uint64 *u64_pointer;
        int64 * i64_pointer;
    #endif//HGL_CPU == HGL_CPU_X86_64

        ValueInterface *value;     //变量
    };//union SystemFuncParam

    struct FuncMap                 //真实函数映射
    {
        void *base;                     //基地址

        void *func;                     //函数地址

        eTokenType result;              //返回类型

        ValueArray<eTokenType> param;         //参数类型

        FuncMap()
        {
            base=0;
            func=0;
        }

        bool Call(const SystemFuncParam *,const int,void *);
    };//struct FuncMap

    struct PropertyMap             //真实属性映射
    {
        eTokenType type;                //数据类型

        void *address;                  //属性地址
    };

    class Command                                                                              //虚拟机指令
    {
    public:

        Command()=default;
        virtual ~Command()=default;

        virtual bool Run(Context *)=0;
    };

    template<typename T> class FuncCall:public Command                                    //函数呼叫
    {
    public:

        T result;                                                                                   //返回值

    public:

        virtual ~FuncCall()=default;

        virtual bool Run(Context *)=0;
    };

//--------------------------------------------------------------------------------------------------
    class ValueInterface                                                                       //变量接口
    {
    protected:

        Module *module;

    public:

        eTokenType type;

    public:

        ValueInterface(Module *dm,eTokenType tt)
        {
            module=dm;
            type=tt;
        }

        virtual ~ValueInterface()=default;
    };

    template<typename T> class Value:public ValueInterface                                //变量
    {
    public:

        virtual T &GetValue()=0;

        void SetValue(T &){};

    public:

        using ValueInterface::ValueInterface;
        virtual ~Value()=default;
    };

    class CompInterface                                                                        //比较基类
    {
    public:

        virtual ~CompInterface()=default;

        virtual bool Comp()=0;
    };

    #ifdef OPER_OVER
    #undef OPER_OVER
    #endif//

    #define OPER_OVER(name,oper)    template<typename T1,typename T2> class name:public CompInterface  \
                                    {   \
                                        Value<T1> *left;   \
                                        Value<T2> *right;  \
                                        \
                                    public: \
                                        \
                                        name(ValueInterface *l,ValueInterface *r) \
                                        {   \
                                            left =(Value<T1> *)l;  \
                                            right=(Value<T2> *)r;  \
                                        }   \
                                        \
                                        ~name() \
                                        {   \
                                            delete left;    \
                                            delete right;   \
                                        }   \
                                        \
                                        bool Comp() override \
                                        {   \
                                            return(left->GetValue() oper right->GetValue());    \
                                        }   \
                                    };

    OPER_OVER(CompEqu,         ==);
    OPER_OVER(CompNotEqu,      !=);
    OPER_OVER(CompLessEqu,     <=);
    OPER_OVER(CompGreaterEqu,  >=);
    OPER_OVER(CompLess,        < );
    OPER_OVER(CompGreater,     > );

    #undef OPER_OVER

    #ifdef DEVIL_VALUE
    #define DEVIL_VALUE
    #endif//DEVIL_VALUE

    #define DEVIL_VALUE(name,T,tt,proc) class name:public Value<T>     \
                                        {   \
                                            T value;    \
                                            \
                                        public: \
                                        \
                                            T &GetValue() override{return value;}    \
                                            \
                                        public: \
                                        \
                                            name(Module *dm,const u16char *str):Value<T>(dm,tt) \
                                            {   \
                                                proc(str,value);    \
                                            }   \
                                        };

    DEVIL_VALUE(Integer,   int,    ttInt,      stoi);              //真实数值,有符号整数
    DEVIL_VALUE(UInteger,  uint,   ttUInt,     stou);              //真实数值,无符号整数
    DEVIL_VALUE(Float,     float,  ttFloat,    stof);              //真实数值,浮点数
    DEVIL_VALUE(Bool,      bool,   ttBool ,    stob);              //真实数值,布尔型
    DEVIL_VALUE(Int64,     int64,  ttInt64,    stoi);
    DEVIL_VALUE(UInt64,    uint64, ttUInt64,   stou);
    DEVIL_VALUE(Double,    double, ttDouble,   stof);

    #undef DEVIL_VALUE

    template<typename T> class ValueProperty:public Value<T>                              //变量：真实属性映射
    {
        T *address;

    public:

        ValueProperty(Module *dm,PropertyMap *dpm,eTokenType type):Value<T>(dm,type)
        {
            address=(T *)(dpm->address);
        }

        T &GetValue() override
        {
            return *address;
        }
    };

    template<typename T> class ValueFuncMap:public Value<T>                               //变量: 函数映射
    {
        Command *cmd;

    public:

        ValueFuncMap(Module *dm,Command *dfc,eTokenType type):Value<T>(dm,type)
        {
            cmd=dfc;
        }

        ~ValueFuncMap()
        {
            delete cmd;
        }

        T &GetValue() override
        {
            cmd->Run(nullptr);

            return ((FuncCall<T> *)cmd)->result;
        }
    };

    template<typename T> class ScriptValue:public Value<T>                                //变量：脚本变量
    {
        U16String func_name;
        U16String value_name;

        T value;

    public:

        ScriptValue()
        {
            func_name=U16_TEXT("null");
            value_name=U16_TEXT("null");
        }

        ScriptValue(Module *dm,const U16String &fn,const U16String &vn,eTokenType tt):Value<T>(dm,tt)
        {
            func_name=fn;
            value_name=vn;
        }

        T &GetValue() override
        {
            return value;
        }

        void SetValue(T &v)
        {
            value=v;
        }
    };
//--------------------------------------------------------------------------------------------------
    template<typename T> class SystemFuncCall:public FuncCall<T>                          //真实函数呼叫
    {
    public:

        virtual ~SystemFuncCall()=default;

        virtual bool Run(Context *)=0;
    };

    template<typename T> class SystemFuncCallFixed:public FuncCall<T>                     //固定参数的真实函数呼叫
    {
        FuncMap *func;             //真实函数映射

        SystemFuncParam *param;
        int param_size;

    public:

        SystemFuncCallFixed(FuncMap *dfm,SystemFuncParam *p,int pc)
        {
            func=dfm;

            param=p;
            param_size=pc*sizeof(SystemFuncParam);
        }

        ~SystemFuncCallFixed()
        {
            delete[] param;
        }

        bool Run(Context *) override
        {
            return func->Call(param,param_size,&(this->result));
        }
    };

    template<typename T> class SystemFuncCallDynamic:public FuncCall<T>                   //可变参数的真实函数呼叫
    {
        FuncMap *func;             //真实函数映射

        SystemFuncParam *param;
        int param_size;

    public:

        SystemFuncCallDynamic(FuncMap *);
        ~SystemFuncCallDynamic();

        bool Run(Context *) override;
    };

    class ScriptFuncCall:public FuncCall<void *>                                          //脚本函数呼叫
    {
        Module *module;
        Func *func;

    public:

        ScriptFuncCall(Module *,Func *);

        bool Run(Context *) override;
    };

    class Goto:public Command                                                             //跳转
    {
        OBJECT_LOGGER

        Module *module;
        Func *func;
        U16String name;

        int index;

    public:

        Goto(Module *,Func *,const U16String &);

        void UpdateGotoFlag();

        bool Run(Context *) override;
    };

    class CompGoto:public Command                                                         //比较并跳转
    {
        OBJECT_LOGGER

        Module *module;
        CompInterface *comp;
        Func *func;

        int index;

    public:

        U16String else_flag;

    public:

        CompGoto(Module *,CompInterface *dci,Func *);
        ~CompGoto();

        void UpdateGotoFlag();

        bool Run(Context *) override;
    };

    class Return:public Command                                                           //函数返回
    {
        Module *module;

    public:

        Return(Module *);

        bool Run(Context *) override;
    };

    class SystemValueEqu:public Command                                                   //真实变量赋值
    {
    public:

        SystemValueEqu(Module *);

        bool Run(Context *) override;
    };

    class ScriptValueEqu:public Command                                                   //脚本变量赋值
    {
    public:

        ScriptValueEqu(Module *);

        bool Run(Context *) override;
    };
}//namespace devil
}//namespace hgl
