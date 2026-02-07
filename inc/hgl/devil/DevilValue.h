#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include "DevilVM/as_tokendef.h"

namespace hgl::devil
{
    using angle_script::eTokenType;

    struct AstValue
    {
        eTokenType type=angle_script::ttVoid;
        std::variant<std::monostate,bool,int32_t,uint32_t,float,std::string> data;

        static AstValue MakeVoid();
        static AstValue MakeBool(bool v);
        static AstValue MakeInt(int32_t v);
        static AstValue MakeUInt(uint32_t v);
        static AstValue MakeFloat(float v);
        static AstValue MakeString(std::string v);

        bool IsNumeric() const;
        bool ToBool() const;
        int32_t ToInt() const;
        uint32_t ToUInt() const;
        float ToFloat() const;
        std::string ToString() const;
    };
}
