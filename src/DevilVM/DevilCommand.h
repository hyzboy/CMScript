#pragma once

#include <string>
#include <hgl/type/Str.Number.h>
#include <vector>
#include"as_tokenizer.h"
#include<hgl/log/Log.h>

namespace hgl::devil
{
    using namespace angle_script;

    class Module;
    class Context;
    class Func;
    class ValueInterface;
    template<typename T> class ValueProperty;
    template<typename T> class ScriptValue;

    union SystemFuncParam          //函数参数
    {
        void *          void_pointer;

        char            c;
        char *          str;
        wchar_t         wc;
        wchar_t *       wide_str;
        u8char          u8c;
        u8char *        u8str;
        u16char         u16c;
        u16char *       u16str;
        u32char         u32c;
        u32char *       u32str;
        int             i;
        int *           int_pointer;
        uint            u;
        uint *          uint_pointer;
        float           f;
        float *         float_pointer;

        double          d;
        double *        double_pointer;
        uint64          ui64;
        int64           si64;
        uint64 *        u64_pointer;
        int64 *         i64_pointer;

        ValueInterface *value;     //变量
    };//union SystemFuncParam

    struct FuncMap                 //真实函数映射
    {
        void *base;                     //基地址

        void *func;                     //函数地址

        eTokenType result;              //返回类型

        std::vector<eTokenType> param;        //参数类型

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
                                            name(Module *dm,const char *str):Value<T>(dm,tt) \
                                            {   \
                                                proc(str,value);    \
                                            }   \
                                        };

    DEVIL_VALUE(ValueInteger,   int,    ttInt,      hgl::stoi);              //真实数值,有符号整数
    DEVIL_VALUE(ValueUInteger,  uint,   ttUInt,     hgl::stou);              //真实数值,无符号整数
    DEVIL_VALUE(ValueFloat,     float,  ttFloat,    hgl::stof);              //真实数值,浮点数
    DEVIL_VALUE(ValueBool,      bool,   ttBool ,    hgl::stob);              //真实数值,布尔型
    DEVIL_VALUE(ValueInt64,     int64,  ttInt64,    hgl::stoi);
    DEVIL_VALUE(ValueUInt64,    uint64, ttUInt64,   hgl::stou);
    DEVIL_VALUE(ValueDouble,    double, ttDouble,   hgl::stof);

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
        std::string func_name;
        std::string value_name;

        T value;

    public:

        ScriptValue()
        {
            func_name="null";
            value_name="null";
        }

        ScriptValue(Module *dm,const std::string &fn,const std::string &vn,eTokenType tt):Value<T>(dm,tt)
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
        std::string name;

        int index;

    public:

        Goto(Module *,Func *,const std::string &);

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

        std::string else_flag;

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
}//namespace hgl::devil
