/*
   AngelCode Scripting Library
   Copyright (c) 2003-2007 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


//
// as_tokendef.h
//
// Definitions for tokens identifiable by the tokenizer
//


#ifndef AS_TOKENDEF_H
#define AS_TOKENDEF_H

#include<hgl/devil/DevilToken.h>

namespace hgl::devil
{
    struct TokenWord
    {
        const char *word;
        TokenType   tokenType;
    };

    constexpr TokenWord tokenWords[] =
    {
        {"+"        , TokenType::Plus},
        {"-"        , TokenType::Minus},
        {"*"        , TokenType::Star},
        {"/"        , TokenType::Slash},
        {"%"        , TokenType::Percent},
        {"="        , TokenType::Assignment},
        {"."        , TokenType::Dot},
        {"+="       , TokenType::AddAssign},
        {"-="       , TokenType::SubAssign},
        {"*="       , TokenType::MulAssign},
        {"/="       , TokenType::DivAssign},
        {"%="       , TokenType::ModAssign},
        {"|="       , TokenType::OrAssign},
        {"&="       , TokenType::AndAssign},
        {"^="       , TokenType::XorAssign},
        {"<<="      , TokenType::ShiftLeftAssign},
        {">>="      , TokenType::ShiftRightLAssign},
        {">>>="     , TokenType::ShiftRightAAssign},
        {"|"        , TokenType::BitOr},
        {"~"        , TokenType::BitNot},
        {"^"        , TokenType::BitXor},
        {"<<"       , TokenType::BitShiftLeft},
        {">>"       , TokenType::BitShiftRight},
        {">>>"      , TokenType::BitShiftRightArith},
        {";"        , TokenType::EndStatement},
        {","        , TokenType::ListSeparator},
        {"{"        , TokenType::StartStatementBlock},
        {"}"        , TokenType::EndStatementBlock},
        {"("        , TokenType::OpenParanthesis},
        {")"        , TokenType::CloseParanthesis},
        {"["        , TokenType::OpenBracket},
        {"]"        , TokenType::CloseBracket},
        {"?"        , TokenType::Question},
        {":"        , TokenType::Colon},
        {"=="       , TokenType::Equal},
        {"!="       , TokenType::NotEqual},
        {"<"        , TokenType::LessThan},
        {">"        , TokenType::GreaterThan},
        {"<="       , TokenType::LessThanOrEqual},
        {">="       , TokenType::GreaterThanOrEqual},
        {"++"       , TokenType::Inc},
        {"--"       , TokenType::Dec},
        {"&"        , TokenType::Amp},
        {"!"        , TokenType::Not},
        {"||"       , TokenType::Or},
        {"&&"       , TokenType::And},
        {"^^"       , TokenType::Xor},
        {"@"        , TokenType::Handle},
        {"and"      , TokenType::And},
        {"bool"     , TokenType::Bool},
        {"break"    , TokenType::Break},
        {"cast"     , TokenType::Cast},
        {"const"    , TokenType::Const},
        {"continue" , TokenType::Continue},
        {"do"       , TokenType::Do},
        {"double"   , TokenType::Double},
        {"else"     , TokenType::Else},
        {"false"    , TokenType::False},
        {"float"    , TokenType::Float},
        {"for"      , TokenType::For},
        {"goto"     , TokenType::Goto},
        {"if"       , TokenType::If},
        {"in"       , TokenType::In},
        {"inout"    , TokenType::InOut},
        {"import"   , TokenType::Import},

        {"enum"     , TokenType::Enum},
        {"func"     , TokenType::Func},
        {"string"   , TokenType::String},

        {"int"      , TokenType::Int},
        {"int8"     , TokenType::Int8},
        {"int16"    , TokenType::Int16},
        {"int32"    , TokenType::Int},
        {"int64"    , TokenType::Int64},
        {"interface", TokenType::Interface},
        {"not"      , TokenType::Not},
        {"null"     , TokenType::Null},
        {"or"       , TokenType::Or},
        {"out"      , TokenType::Out},
        {"return"   , TokenType::Return},
        {"true"     , TokenType::True},
        {"void"     , TokenType::Void},
        {"while"    , TokenType::While},
        {"uint"     , TokenType::UInt},
        {"uint8"    , TokenType::UInt8},
        {"uint16"   , TokenType::UInt16},
        {"uint32"   , TokenType::UInt},
        {"uint64"   , TokenType::UInt64},
        {"switch"   , TokenType::Switch},
        {"class"    , TokenType::Class},
        {"case"     , TokenType::Case},
        {"CASE"     , TokenType::Case},
        {"default"  , TokenType::Default},
        {"xor"      , TokenType::Xor},
    };

    constexpr int   numTokenWords   =sizeof(tokenWords)/sizeof(TokenWord);

    constexpr char  whiteSpace[]    =" \t\r\n\xEF\xBB\xBF";
    constexpr int   whiteSpaceNumber=sizeof(whiteSpace)/sizeof(whiteSpace[0])-1;
}//namespace hgl::devil
#endif

