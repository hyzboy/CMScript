#include <hgl/devil/DevilBytecode.h>
#include <cstring>

namespace hgl::devil
{
    namespace
    {
        bool ComputeFastInt(BytecodeFunction &func)
        {
            func.const_ints.clear();

            for(const AstValue &value:func.constants)
            {
                if(value.type!=TokenType::Bool && value.type!=TokenType::Int
                    && value.type!=TokenType::Int8 && value.type!=TokenType::Int16)
                {
                    return false;
                }
                func.const_ints.push_back(value.ToInt());
            }

            for(const Instruction &ins:func.code)
            {
                switch(ins.op)
                {
                    case OpCode::Nop:
                    case OpCode::PushConst:
                    case OpCode::Pop:
                    case OpCode::LoadLocal:
                    case OpCode::StoreLocal:
                    case OpCode::AddLocalConst:
                    case OpCode::SubLocalConst:
                    case OpCode::JumpIfLocalGeConst:
                    case OpCode::Add:
                    case OpCode::Sub:
                    case OpCode::Mul:
                    case OpCode::Div:
                    case OpCode::Mod:
                    case OpCode::Neg:
                    case OpCode::Not:
                    case OpCode::BitNot:
                    case OpCode::BitAnd:
                    case OpCode::BitOr:
                    case OpCode::BitXor:
                    case OpCode::Shl:
                    case OpCode::Shr:
                    case OpCode::Eq:
                    case OpCode::Ne:
                    case OpCode::Lt:
                    case OpCode::Le:
                    case OpCode::Gt:
                    case OpCode::Ge:
                    case OpCode::And:
                    case OpCode::Or:
                    case OpCode::Jump:
                    case OpCode::JumpIfFalse:
                    case OpCode::Ret:
                        break;
                    case OpCode::Cast:
                    {
                        const TokenType target=static_cast<TokenType>(ins.a);
                        if(target!=TokenType::Bool && target!=TokenType::Int
                            && target!=TokenType::Int8 && target!=TokenType::Int16)
                            return false;
                        break;
                    }
                    default:
                        return false;
                }
            }

            return true;
        }
        void WriteBytes(std::vector<uint8_t> &out,const void *data,size_t size)
        {
            const uint8_t *ptr=static_cast<const uint8_t *>(data);
            out.insert(out.end(),ptr,ptr+size);
        }

        bool ReadBytes(const std::vector<uint8_t> &data,size_t &offset,void *out,size_t size)
        {
            if(offset+size>data.size())
                return false;
            std::memcpy(out,data.data()+offset,size);
            offset+=size;
            return true;
        }

        bool WriteString(std::vector<uint8_t> &out,const std::string &value)
        {
            const uint32_t len=static_cast<uint32_t>(value.size());
            WriteBytes(out,&len,sizeof(len));
            if(len>0)
                WriteBytes(out,value.data(),len);
            return true;
        }

        bool ReadString(const std::vector<uint8_t> &data,size_t &offset,std::string &out_value)
        {
            uint32_t len=0;
            if(!ReadBytes(data,offset,&len,sizeof(len)))
                return false;
            if(offset+len>data.size())
                return false;
            out_value.assign(reinterpret_cast<const char *>(data.data()+offset),len);
            offset+=len;
            return true;
        }

        bool WriteValue(std::vector<uint8_t> &out,const AstValue &value)
        {
            const int32_t type=static_cast<int32_t>(value.type);
            WriteBytes(out,&type,sizeof(type));

            switch(value.type)
            {
                case TokenType::Bool:
                {
                    const uint8_t v=value.ToBool()?1u:0u;
                    WriteBytes(out,&v,sizeof(v));
                    return true;
                }
                case TokenType::Int:
                case TokenType::Int8:
                case TokenType::Int16:
                {
                    const int32_t v=value.ToInt();
                    WriteBytes(out,&v,sizeof(v));
                    return true;
                }
                case TokenType::UInt:
                case TokenType::UInt8:
                case TokenType::UInt16:
                {
                    const uint32_t v=value.ToUInt();
                    WriteBytes(out,&v,sizeof(v));
                    return true;
                }
                case TokenType::Float:
                {
                    const float v=value.ToFloat();
                    WriteBytes(out,&v,sizeof(v));
                    return true;
                }
                case TokenType::String:
                {
                    return WriteString(out,value.ToString());
                }
                case TokenType::Void:
                    return true;
                default:
                    return false;
            }
        }

        bool ReadValue(const std::vector<uint8_t> &data,size_t &offset,AstValue &out_value)
        {
            int32_t type=0;
            if(!ReadBytes(data,offset,&type,sizeof(type)))
                return false;

            const TokenType tt=static_cast<TokenType>(type);
            switch(tt)
            {
                case TokenType::Bool:
                {
                    uint8_t v=0;
                    if(!ReadBytes(data,offset,&v,sizeof(v)))
                        return false;
                    out_value=AstValue::MakeBool(v!=0);
                    return true;
                }
                case TokenType::Int:
                case TokenType::Int8:
                case TokenType::Int16:
                {
                    int32_t v=0;
                    if(!ReadBytes(data,offset,&v,sizeof(v)))
                        return false;
                    out_value=AstValue::MakeInt(v);
                    return true;
                }
                case TokenType::UInt:
                case TokenType::UInt8:
                case TokenType::UInt16:
                {
                    uint32_t v=0;
                    if(!ReadBytes(data,offset,&v,sizeof(v)))
                        return false;
                    out_value=AstValue::MakeUInt(v);
                    return true;
                }
                case TokenType::Float:
                {
                    float v=0.0f;
                    if(!ReadBytes(data,offset,&v,sizeof(v)))
                        return false;
                    out_value=AstValue::MakeFloat(v);
                    return true;
                }
                case TokenType::String:
                {
                    std::string s;
                    if(!ReadString(data,offset,s))
                        return false;
                    out_value=AstValue::MakeString(std::move(s));
                    return true;
                }
                case TokenType::Void:
                    out_value=AstValue::MakeVoid();
                    return true;
                default:
                    return false;
            }
        }
    }

    bool BytecodeModule::Serialize(std::vector<uint8_t> &out_data) const
    {
        out_data.clear();
        const uint32_t magic=0x4D434244; // DBCM
        const uint32_t version=2;
        WriteBytes(out_data,&magic,sizeof(magic));
        WriteBytes(out_data,&version,sizeof(version));

        const uint32_t func_count=static_cast<uint32_t>(functions.size());
        WriteBytes(out_data,&func_count,sizeof(func_count));

        for(const auto &kv:functions)
        {
            const BytecodeFunction &func=kv.second;
            WriteString(out_data,func.name);
            WriteBytes(out_data,&func.local_count,sizeof(func.local_count));
            WriteBytes(out_data,&func.param_count,sizeof(func.param_count));

            const uint32_t const_count=static_cast<uint32_t>(func.constants.size());
            WriteBytes(out_data,&const_count,sizeof(const_count));
            for(const AstValue &v:func.constants)
                if(!WriteValue(out_data,v))
                    return false;

            const uint32_t code_count=static_cast<uint32_t>(func.code.size());
            WriteBytes(out_data,&code_count,sizeof(code_count));
            for(const Instruction &ins:func.code)
            {
                const uint8_t op=static_cast<uint8_t>(ins.op);
                WriteBytes(out_data,&op,sizeof(op));
                WriteBytes(out_data,&ins.a,sizeof(ins.a));
                WriteBytes(out_data,&ins.b,sizeof(ins.b));
                WriteBytes(out_data,&ins.c,sizeof(ins.c));
            }
        }

        return true;
    }

    bool BytecodeModule::Deserialize(const std::vector<uint8_t> &data)
    {
        size_t offset=0;
        uint32_t magic=0;
        uint32_t version=0;

        if(!ReadBytes(data,offset,&magic,sizeof(magic)))
            return false;
        if(!ReadBytes(data,offset,&version,sizeof(version)))
            return false;
        if(magic!=0x4D434244 || (version!=1 && version!=2))
            return false;

        uint32_t func_count=0;
        if(!ReadBytes(data,offset,&func_count,sizeof(func_count)))
            return false;

        functions.clear();
        for(uint32_t i=0;i<func_count;++i)
        {
            BytecodeFunction func;
            if(!ReadString(data,offset,func.name))
                return false;

            if(!ReadBytes(data,offset,&func.local_count,sizeof(func.local_count)))
                return false;
            if(!ReadBytes(data,offset,&func.param_count,sizeof(func.param_count)))
                return false;

            uint32_t const_count=0;
            if(!ReadBytes(data,offset,&const_count,sizeof(const_count)))
                return false;
            func.constants.reserve(const_count);
            for(uint32_t c=0;c<const_count;++c)
            {
                AstValue v;
                if(!ReadValue(data,offset,v))
                    return false;
                func.constants.push_back(std::move(v));
            }

            uint32_t code_count=0;
            if(!ReadBytes(data,offset,&code_count,sizeof(code_count)))
                return false;
            func.code.reserve(code_count);
            for(uint32_t c=0;c<code_count;++c)
            {
                uint8_t op=0;
                Instruction ins;
                if(!ReadBytes(data,offset,&op,sizeof(op)))
                    return false;
                ins.op=static_cast<OpCode>(op);
                if(!ReadBytes(data,offset,&ins.a,sizeof(ins.a)))
                    return false;
                if(!ReadBytes(data,offset,&ins.b,sizeof(ins.b)))
                    return false;
                if(!ReadBytes(data,offset,&ins.c,sizeof(ins.c)))
                    return false;
                func.code.push_back(ins);
            }

            func.fast_int=ComputeFastInt(func);
            functions.emplace(func.name,std::move(func));
        }

        return true;
    }
}
