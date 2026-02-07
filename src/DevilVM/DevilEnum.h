#pragma once

#include<hgl/type/String.h>
#include<hgl/type/UnorderedMap.h>
namespace hgl
{
namespace devil
{
    /**
    * 枚举定义基类<br>
    * 用于在DevilEngine中定义
    */
    class Enum                                                                                 ///枚举定义基类,用于在DevilEngine中定义
    {
    public:

        virtual bool Add(const U16String &);

        virtual uint32 GetValue32(const U16String &);                                             ///<取得相应枚举值在内存中的32位数据
    };

    template<typename T>class EnumTypedef:public Enum                                     ///枚举数据类型定义基类
    {
        UnorderedMap<U16String,T> Items;                                                               ///<枚举项名字

    public:

        virtual bool Add(const U16String &);
    };

    template<typename T>class EnumSigned:public EnumTypedef<T>                            ///有符号整数型枚举定义基类
    {
    public:

        virtual bool Add(const U16String &);
    };

    template<typename T>class EnumUnsigned:public EnumTypedef<T>                          ///无符号整数型枚举定义基类
    {
    public:

        virtual bool Add(const U16String &);
    };

    template<typename T>class EnumFloat:public EnumTypedef<T>                             ///浮点数枚举定义基类
    {
    public:

        virtual bool Add(const U16String &);
    };

    template<typename T>class EnumString:public EnumTypedef<T>                            ///字符串枚举定义基类
    {
    public:

        virtual bool Add(const U16String &);
    };
}//namespace devil
}//namespace hgl
