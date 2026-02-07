#include <hgl/devil/DevilBytecode.h>
#include <sstream>

namespace hgl::devil
{
    namespace
    {
        std::string FormatValue(const AstValue &value)
        {
            switch(value.type)
            {
                case TokenType::Bool: return value.ToBool()?"true":"false";
                case TokenType::Int:
                case TokenType::Int8:
                case TokenType::Int16: return std::to_string(value.ToInt());
                case TokenType::UInt:
                case TokenType::UInt8:
                case TokenType::UInt16: return std::to_string(value.ToUInt());
                case TokenType::Float: return std::to_string(value.ToFloat());
                case TokenType::String: return "\""+value.ToString()+"\"";
                case TokenType::Void: return "void";
                default: return "?";
            }
        }
    }

    const char *OpCodeToString(OpCode op)
    {
        switch(op)
        {
            case OpCode::Nop: return "Nop";
            case OpCode::PushConst: return "PushConst";
            case OpCode::Pop: return "Pop";
            case OpCode::LoadLocal: return "LoadLocal";
            case OpCode::StoreLocal: return "StoreLocal";
            case OpCode::Add: return "Add";
            case OpCode::Sub: return "Sub";
            case OpCode::Mul: return "Mul";
            case OpCode::Div: return "Div";
            case OpCode::Mod: return "Mod";
            case OpCode::Neg: return "Neg";
            case OpCode::Not: return "Not";
            case OpCode::BitNot: return "BitNot";
            case OpCode::BitAnd: return "BitAnd";
            case OpCode::BitOr: return "BitOr";
            case OpCode::BitXor: return "BitXor";
            case OpCode::Shl: return "Shl";
            case OpCode::Shr: return "Shr";
            case OpCode::Eq: return "Eq";
            case OpCode::Ne: return "Ne";
            case OpCode::Lt: return "Lt";
            case OpCode::Le: return "Le";
            case OpCode::Gt: return "Gt";
            case OpCode::Ge: return "Ge";
            case OpCode::And: return "And";
            case OpCode::Or: return "Or";
            case OpCode::Jump: return "Jump";
            case OpCode::JumpIfFalse: return "JumpIfFalse";
            case OpCode::CallNative: return "CallNative";
            case OpCode::CallFunc: return "CallFunc";
            case OpCode::Ret: return "Ret";
            default: return "Unknown";
        }
    }

    std::string DisassembleInstruction(const BytecodeFunction &func,size_t index)
    {
        if(index>=func.code.size())
            return std::string();

        const Instruction &ins=func.code[index];
        std::ostringstream out;

        out<<index<<": "<<OpCodeToString(ins.op);

        switch(ins.op)
        {
            case OpCode::PushConst:
                if(ins.a>=0 && static_cast<size_t>(ins.a)<func.constants.size())
                    out<<" "<<ins.a<<" ("<<FormatValue(func.constants[static_cast<size_t>(ins.a)])<<")";
                else
                    out<<" "<<ins.a;
                break;
            case OpCode::LoadLocal:
            case OpCode::StoreLocal:
            case OpCode::Jump:
            case OpCode::JumpIfFalse:
                out<<" "<<ins.a;
                break;
            case OpCode::CallNative:
            case OpCode::CallFunc:
                out<<" "<<ins.a<<","<<ins.b;
                if(ins.a>=0 && static_cast<size_t>(ins.a)<func.constants.size())
                    out<<" ("<<func.constants[static_cast<size_t>(ins.a)].ToString()<<")";
                break;
            default:
                break;
        }

        return out.str();
    }

    std::string DisassembleFunction(const BytecodeFunction &func)
    {
        std::ostringstream out;
        out<<"func "<<func.name<<" locals="<<func.local_count<<" params="<<func.param_count<<"\n";
        for(size_t i=0;i<func.code.size();++i)
            out<<DisassembleInstruction(func,i)<<"\n";
        return out.str();
    }

    std::string DisassembleModule(const BytecodeModule &module)
    {
        std::ostringstream out;
        const auto &funcs=module.GetFunctions();
        for(const auto &item:funcs)
            out<<DisassembleFunction(item.second)<<"\n";
        return out.str();
    }
}
