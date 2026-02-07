#pragma once

#include <string>
#include <list>
#include <ankerl/unordered_dense.h>
#include <hgl/log/Log.h>
#include <hgl/platform/compiler/EventFunc.h>

namespace hgl::devil
{
    class Func;
    class EnumDef;
    struct PropertyMap;
    struct FuncMap;

    /**
     * 虚拟机处理模块
     */
    class Module
    {
        OBJECT_LOGGER

        ankerl::unordered_dense::map<std::string,PropertyMap *>   prop_map;       //属性映射表
        ankerl::unordered_dense::map<std::string,FuncMap *>       func_map;       //函数映射表
        ankerl::unordered_dense::map<std::string,Func *>          script_func;    //脚本函数表
        ankerl::unordered_dense::map<std::string,EnumDef *>       enum_map;       //枚举映射表

    private:

        bool _MapFunc(const char *,void *,void *);

    public: //事件

        DefEvent(bool,OnTrueFuncCall,(const char *));                           ///<真实函数呼叫

    public:

        std::list<std::string> string_list;                           //字符串列表

    public:

        Module(){OnTrueFuncCall=nullptr;}
        virtual ~Module()=default;

        Func *GetScriptFunc(const std::string &);
        FuncMap *GetFuncMap(const std::string &);
        PropertyMap *GetPropertyMap(const std::string &);

        virtual bool MapProperty(const char *,void *);                         ///<映射属性(真实变量的映射，在整个模块中全局有效)
        virtual bool MapFunc(const char *,void *);                             ///<映射C函数
    //      virtual bool MapFunc(void *,const char *,void *);                      ///<映射C函数,并传入一个对像
        virtual bool MapFunc(const char *,void *,void *);                      ///<映射C++成员函数

        virtual bool AddScript(const char *,int=-1);                           ///<添加脚本并编译

        virtual bool AddEnum(const char *,EnumDef *);

        virtual void Clear();                                                  ///<清除所有模块和映射

    public: //调试用函数

        #ifdef _DEBUG

        virtual void LogPropertyList();                                        ///<输出属性变量列表
        virtual void LogMapFuncList();                                         ///<输出映射函数列表
        virtual void LogScriptFuncList();                                      ///<输出脚本函数列表

        #endif//_DEBUG
    };//class Module
}//namespace hgl::devil
