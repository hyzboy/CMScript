#pragma once

#include <string>
#include <cstdint>
#include <ankerl/unordered_dense.h>

namespace hgl::devil
{
    /**
    * 枚举定义基类<br>
    * 用于在DevilEngine中定义
    */
    class EnumDef                                                                              ///枚举定义基类,用于在DevilEngine中定义
    {
    public:

        virtual bool Add(const std::string &);

        virtual uint32_t GetValue32(const std::string &);                                         ///<取得相应枚举值在内存中的32位数据
    };

    template<typename T>class EnumTypedef:public EnumDef                                  ///枚举数据类型定义基类
    {
        ankerl::unordered_dense::map<std::string,T> Items;                                             ///<枚举项名字

    public:

        virtual bool Add(const std::string &);
    };

    template<typename T>class EnumSigned:public EnumTypedef<T>                            ///有符号整数型枚举定义基类
    {
    public:

        virtual bool Add(const std::string &);
    };

    template<typename T>class EnumUnsigned:public EnumTypedef<T>                          ///无符号整数型枚举定义基类
    {
    public:

        virtual bool Add(const std::string &);
    };

    template<typename T>class EnumFloat:public EnumTypedef<T>                             ///浮点数枚举定义基类
    {
    public:

        virtual bool Add(const std::string &);
    };

    template<typename T>class EnumString:public EnumTypedef<T>                            ///字符串枚举定义基类
    {
    public:

        virtual bool Add(const std::string &);
    };
}//namespace hgl::devil
