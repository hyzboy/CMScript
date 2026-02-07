#pragma once

#include <cstdint>
#include <string>
#include <string>
#include <hgl/devil/DevilToken.h>

namespace hgl::devil
{
    struct AstValue
    {
        TokenType type=TokenType::Void;
        union
        {
            bool b;
            int32_t i;
            uint32_t u;
            float f;
            double d;
        } data{0};
        std::string s;

        static AstValue MakeVoid();
        static AstValue MakeBool(bool v);
        static AstValue MakeInt(int32_t v);
        static AstValue MakeUInt(uint32_t v);
        static AstValue MakeFloat(float v);
        static AstValue MakeDouble(double v);
        static AstValue MakeString(std::string v);

        bool IsNumeric() const;
        bool ToBool() const;
        int32_t ToInt() const;
        uint32_t ToUInt() const;
        float ToFloat() const;
        double ToDouble() const;
        std::string ToString() const;
    };
}


