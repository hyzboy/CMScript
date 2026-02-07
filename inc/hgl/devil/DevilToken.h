#pragma once

namespace hgl::devil
{
    enum class TokenType
    {
        UnrecognizedToken,

        End,                 // End of file

        // White space and comments
        WhiteSpace,          // ' ', '\t', '\r', '\n'
        OnelineComment,      // // \n
        MultilineComment,    // /* */

        // Atoms
        Identifier,            // abc123
        IntConstant,           // 1234
        FloatConstant,         // 12.34e56f
        DoubleConstant,        // 12.34e56
        StringConstant,        // "123"
        HeredocStringConstant, // """text"""
        NonTerminatedStringConstant, // "123
        BitsConstant,          // 0xFFFF

        // Math operators
        Plus,                // +
        Minus,               // -
        Star,                // *
        Slash,               // /
        Percent,             // %

        Handle,              // #

        AddAssign,           // +=
        SubAssign,           // -=
        MulAssign,           // *=
        DivAssign,           // /=
        ModAssign,           // %=

        OrAssign,            // |=
        AndAssign,           // &=
        XorAssign,           // ^=
        ShiftLeftAssign,     // <<=
        ShiftRightLAssign,   // >>=
        ShiftRightAAssign,   // >>>=

        Inc,                 // ++
        Dec,                 // --

        Dot,                 // .

        // Statement tokens
        Assignment,          // =
        EndStatement,        // ;
        ListSeparator,       // ,
        StartStatementBlock, // {
        EndStatementBlock,   // }
        OpenParanthesis,     // (
        CloseParanthesis,    // )
        OpenBracket,         // [
        CloseBracket,        // ]
        Amp,                 // &

        // Bitwise operators
        BitOr,               // |
        BitNot,              // ~
        BitXor,              // ^
        BitShiftLeft,        // <<
        BitShiftRight,       // >>
        BitShiftRightArith,  // >>>

        // Compare operators
        Equal,               // ==
        NotEqual,            // !=
        LessThan,            // <
        GreaterThan,         // >
        LessThanOrEqual,     // <=
        GreaterThanOrEqual,  // >=

        Question,            // ?
        Colon,               // :

        // Reserved keywords
        If,                  // if
        Else,                // else
        For,                 // for
        While,               // while
        Bool,                // bool
        Import,              // import

        Goto,                // goto

        Enum,                // enum
        Func,                // function
        String,              // string

        Int,                 // int
        Int8,                // int8
        Int16,               // int16
        Int64,               // int64
        Interface,           // interface
        UInt,                // uint
        UInt8,               // uint8
        UInt16,              // uint16
        UInt64,              // uint64
        Float,               // float
        Void,                // void
        True,                // true
        False,               // false
        Return,              // return
        Not,                 // not
        And,                 // and
        Or,                  // or
        Xor,                 // xor
        Break,               // break
        Continue,            // continue
        Const,               // const
        Do,                  // do
        Double,              // double
        Switch,              // switch
        Case,                // case
        Default,             // default
        In,                  // in
        Out,                 // out
        InOut,               // inout
        Null,                // null
        Class,               // class
        Cast                 // cast
    };

};//namespace hgl::devil
