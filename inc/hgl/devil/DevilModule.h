#pragma once

#include <string>
#include <list>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <type_traits>
#include <ankerl/unordered_dense.h>
#include <hgl/log/Log.h>
#include <hgl/platform/compiler/EventFunc.h>

namespace hgl::devil
{
    namespace detail
    {
        enum class BindType
        {
            Void,
            Bool,
            Int,
            Int8,
            Int16,
            UInt,
            UInt8,
            UInt16,
            Float,
            String
        };

        template<typename>
        struct DependentFalse : std::false_type
        {
        };

        template<typename T>
        struct BindTypeTraits
        {
            static_assert(DependentFalse<T>::value, "DevilScript MapFunc: unsupported parameter type.");
        };

        template<> struct BindTypeTraits<void>          { static constexpr BindType value = BindType::Void; };
        template<> struct BindTypeTraits<bool>          { static constexpr BindType value = BindType::Bool; };
        template<> struct BindTypeTraits<int>           { static constexpr BindType value = BindType::Int; };
        template<> struct BindTypeTraits<std::int8_t>   { static constexpr BindType value = BindType::Int8; };
        template<> struct BindTypeTraits<std::int16_t>  { static constexpr BindType value = BindType::Int16; };
        template<> struct BindTypeTraits<unsigned int>  { static constexpr BindType value = BindType::UInt; };
        template<> struct BindTypeTraits<std::uint8_t>  { static constexpr BindType value = BindType::UInt8; };
        template<> struct BindTypeTraits<std::uint16_t> { static constexpr BindType value = BindType::UInt16; };
        template<> struct BindTypeTraits<float>         { static constexpr BindType value = BindType::Float; };
        template<> struct BindTypeTraits<char *>        { static constexpr BindType value = BindType::String; };
        template<> struct BindTypeTraits<const char *>  { static constexpr BindType value = BindType::String; };

        template<typename T>
        constexpr BindType BindTypeOf()
        {
            return BindTypeTraits<std::decay_t<T>>::value;
        }
    }//namespace detail

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
        bool _MapFuncTyped(const char *,void *,void *,detail::BindType,std::initializer_list<detail::BindType>);

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

        template<typename R,typename... Args>
        bool MapFunc(const char *name,R (*func)(Args...))
        {
            return _MapFuncTyped(name,nullptr,reinterpret_cast<void *>(func),detail::BindTypeOf<R>(),{detail::BindTypeOf<Args>()...});
        }

        template<typename C,typename R,typename... Args>
        bool MapFunc(const char *name,C *instance,R (C::*func)(Args...))
        {
            void *func_ptr=nullptr;

            static_assert(sizeof(func)==sizeof(func_ptr),"DevilScript MapFunc: member function pointer ABI is not supported.");

            std::memcpy(&func_ptr,&func,sizeof(func_ptr));

            return _MapFuncTyped(name,instance,func_ptr,detail::BindTypeOf<R>(),{detail::BindTypeOf<Args>()...});
        }

        template<typename C,typename R,typename... Args>
        bool MapFunc(const char *name,const C *instance,R (C::*func)(Args...) const)
        {
            void *func_ptr=nullptr;

            static_assert(sizeof(func)==sizeof(func_ptr),"DevilScript MapFunc: member function pointer ABI is not supported.");

            std::memcpy(&func_ptr,&func,sizeof(func_ptr));

            return _MapFuncTyped(name,const_cast<C *>(instance),func_ptr,detail::BindTypeOf<R>(),{detail::BindTypeOf<Args>()...});
        }

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
