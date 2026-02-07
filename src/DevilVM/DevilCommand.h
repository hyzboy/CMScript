#pragma once

#include<hgl/type/String.h>
#include<hgl/type/ValueArray.h>
#include"as_tokenizer.h"
#include<hgl/log/Log.h>

using namespace angle_script;
namespace hgl
{
    class DevilModule;
    class DevilContext;
    class DevilFunc;
    class DevilValueInterface;
    template<typename T> class DevilValueProperty;
    template<typename T> class DevilScriptValue;

    union DevilSystemFuncParam          //函数参数
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

        DevilValueInterface *value;     //变量
    };//union DevilSystemFuncParam

    struct DevilFuncMap                 //真实函数映射
    {
        void *base;                     //基地址

        void *func;                     //函数地址

        eTokenType result;              //返回类型

        ValueArray<eTokenType> param;         //参数类型

        DevilFuncMap()
        {
            base=0;
            func=0;
        }

        bool Call(const DevilSystemFuncParam *,const int,void *);
    };//struct DevilFuncMap

    struct DevilPropertyMap             //真实属性映射
    {
        eTokenType type;                //数据类型

        void *address;                  //属性地址
    };

    class DevilCommand                                                                              //虚拟机指令
    {
    public:

        DevilCommand()=default;
        virtual ~DevilCommand()=default;

        virtual bool Run(DevilContext *)=0;
    };

    template<typename T> class DevilFuncCall:public DevilCommand                                    //函数呼叫
    {
    public:

        T result;                                                                                   //返回值

    public:

        virtual ~DevilFuncCall()=default;

        virtual bool Run(DevilContext *)=0;
    };

//--------------------------------------------------------------------------------------------------
    class DevilValueInterface                                                                       //变量接口
    {
    protected:

        DevilModule *module;

    public:

        eTokenType type;

    public:

        DevilValueInterface(DevilModule *dm,eTokenType tt)
        {
            module=dm;
            type=tt;
        }

        virtual ~DevilValueInterface()=default;
    };

    template<typename T> class DevilValue:public DevilValueInterface                                //变量
    {
    public:

        virtual T &GetValue()=0;

        void SetValue(T &){};

    public:

        using DevilValueInterface::DevilValueInterface;
        virtual ~DevilValue()=default;
    };

    class DevilCompInterface                                                                        //比较基类
    {
    public:

        virtual ~DevilCompInterface()=default;

        virtual bool Comp()=0;
    };

    #ifdef OPER_OVER
    #undef OPER_OVER
    #endif//

    #define OPER_OVER(name,oper)    template<typename T1,typename T2> class name:public DevilCompInterface  \
                                    {   \
                                        DevilValue<T1> *left;   \
                                        DevilValue<T2> *right;  \
                                        \
                                    public: \
                                        \
                                        name(DevilValueInterface *l,DevilValueInterface *r) \
                                        {   \
                                            left =(DevilValue<T1> *)l;  \
                                            right=(DevilValue<T2> *)r;  \
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

    OPER_OVER(DevilCompEqu,         ==);
    OPER_OVER(DevilCompNotEqu,      !=);
    OPER_OVER(DevilCompLessEqu,     <=);
    OPER_OVER(DevilCompGreaterEqu,  >=);
    OPER_OVER(DevilCompLess,        < );
    OPER_OVER(DevilCompGreater,     > );

    #undef OPER_OVER

    #ifdef DEVIL_VALUE
    #define DEVIL_VALUE
    #endif//DEVIL_VALUE

    #define DEVIL_VALUE(name,T,tt,proc) class name:public DevilValue<T>     \
                                        {   \
                                            T value;    \
                                            \
                                        public: \
                                        \
                                            T &GetValue() override{return value;}    \
                                            \
                                        public: \
                                        \
                                            name(DevilModule *dm,const u16char *str):DevilValue<T>(dm,tt) \
                                            {   \
                                                proc(str,value);    \
                                            }   \
                                        };

    DEVIL_VALUE(DevilInteger,   int,    ttInt,      stoi);              //真实数值,有符号整数
    DEVIL_VALUE(DevilUInteger,  uint,   ttUInt,     stou);              //真实数值,无符号整数
    DEVIL_VALUE(DevilFloat,     float,  ttFloat,    stof);              //真实数值,浮点数
    DEVIL_VALUE(DevilBool,      bool,   ttBool ,    stob);              //真实数值,布尔型
    DEVIL_VALUE(DevilInt64,     int64,  ttInt64,    stoi);
    DEVIL_VALUE(DevilUInt64,    uint64, ttUInt64,   stou);
    DEVIL_VALUE(DevilDouble,    double, ttDouble,   stof);

    #undef DEVIL_VALUE

    template<typename T> class DevilValueProperty:public DevilValue<T>                              //变量：真实属性映射
    {
        T *address;

    public:

        DevilValueProperty(DevilModule *dm,DevilPropertyMap *dpm,eTokenType type):DevilValue<T>(dm,type)
        {
            address=(T *)(dpm->address);
        }

        T &GetValue() override
        {
            return *address;
        }
    };

    template<typename T> class DevilValueFuncMap:public DevilValue<T>                               //变量: 函数映射
    {
        DevilCommand *cmd;

    public:

        DevilValueFuncMap(DevilModule *dm,DevilCommand *dfc,eTokenType type):DevilValue<T>(dm,type)
        {
            cmd=dfc;
        }

        ~DevilValueFuncMap()
        {
            delete cmd;
        }

        T &GetValue() override
        {
            cmd->Run(nullptr);

            return ((DevilFuncCall<T> *)cmd)->result;
        }
    };

    template<typename T> class DevilScriptValue:public DevilValue<T>                                //变量：脚本变量
    {
        U16String func_name;
        U16String value_name;

        T value;

    public:

        DevilScriptValue()
        {
            func_name=U16_TEXT("null");
            value_name=U16_TEXT("null");
        }

        DevilScriptValue(DevilModule *dm,const U16String &fn,const U16String &vn,eTokenType tt):DevilValue<T>(dm,tt)
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
    template<typename T> class DevilSystemFuncCall:public DevilFuncCall<T>                          //真实函数呼叫
    {
    public:

        virtual ~DevilSystemFuncCall()=default;

        virtual bool Run(DevilContext *)=0;
    };

    template<typename T> class DevilSystemFuncCallFixed:public DevilFuncCall<T>                     //固定参数的真实函数呼叫
    {
        DevilFuncMap *func;             //真实函数映射

        DevilSystemFuncParam *param;
        int param_size;

    public:

        DevilSystemFuncCallFixed(DevilFuncMap *dfm,DevilSystemFuncParam *p,int pc)
        {
            func=dfm;

            param=p;
            param_size=pc*sizeof(DevilSystemFuncParam);
        }

        ~DevilSystemFuncCallFixed()
        {
            delete[] param;
        }

        bool Run(DevilContext *) override
        {
            return func->Call(param,param_size,&(this->result));
        }
    };

    template<typename T> class DevilSystemFuncCallDynamic:public DevilFuncCall<T>                   //可变参数的真实函数呼叫
    {
        DevilFuncMap *func;             //真实函数映射

        DevilSystemFuncParam *param;
        int param_size;

    public:

        DevilSystemFuncCallDynamic(DevilFuncMap *);
        ~DevilSystemFuncCallDynamic();

        bool Run(DevilContext *) override;
    };

    class DevilScriptFuncCall:public DevilFuncCall<void *>                                          //脚本函数呼叫
    {
        DevilModule *module;
        DevilFunc *func;

    public:

        DevilScriptFuncCall(DevilModule *,DevilFunc *);

        bool Run(DevilContext *) override;
    };

    class DevilGoto:public DevilCommand                                                             //跳转
    {
        OBJECT_LOGGER

        DevilModule *module;
        DevilFunc *func;
        U16String name;

        int index;

    public:

        DevilGoto(DevilModule *,DevilFunc *,const U16String &);

        void UpdateGotoFlag();

        bool Run(DevilContext *) override;
    };

    class DevilCompGoto:public DevilCommand                                                         //比较并跳转
    {
        OBJECT_LOGGER

        DevilModule *module;
        DevilCompInterface *comp;
        DevilFunc *func;

        int index;

    public:

        U16String else_flag;

    public:

        DevilCompGoto(DevilModule *,DevilCompInterface *dci,DevilFunc *);
        ~DevilCompGoto();

        void UpdateGotoFlag();

        bool Run(DevilContext *) override;
    };

    class DevilReturn:public DevilCommand                                                           //函数返回
    {
        DevilModule *module;

    public:

        DevilReturn(DevilModule *);

        bool Run(DevilContext *) override;
    };

    class DevilSystemValueEqu:public DevilCommand                                                   //真实变量赋值
    {
    public:

        DevilSystemValueEqu(DevilModule *);

        bool Run(DevilContext *) override;
    };

    class DevilScriptValueEqu:public DevilCommand                                                   //脚本变量赋值
    {
    public:

        DevilScriptValueEqu(DevilModule *);

        bool Run(DevilContext *) override;
    };
}//namespace hgl
