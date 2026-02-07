#include <hgl/devil/DevilBytecode.h>
#include <hgl/devil/DevilModule.h>
#include "DevilCommand.h"
#include <cmath>

namespace hgl::devil
{
    bool CallNativeFunc(FuncMap *map,const std::vector<AstValue> &args,Module &module,AstValue &out_value);

    namespace
    {
        AstValue PopValue(std::vector<AstValue> &stack,bool &ok)
        {
            if(stack.empty())
            {
                ok=false;
                return AstValue::MakeVoid();
            }

            AstValue v=std::move(stack.back());
            stack.pop_back();
            ok=true;
            return v;
        }

        AstValue BinaryNumeric(const AstValue &lhs,const AstValue &rhs,OpCode op)
        {
            const bool use_float=(lhs.type==TokenType::Float || rhs.type==TokenType::Float);
            if(use_float)
            {
                const float a=lhs.ToFloat();
                const float b=rhs.ToFloat();
                switch(op)
                {
                    case OpCode::Add: return AstValue::MakeFloat(a+b);
                    case OpCode::Sub: return AstValue::MakeFloat(a-b);
                    case OpCode::Mul: return AstValue::MakeFloat(a*b);
                    case OpCode::Div: return AstValue::MakeFloat(b!=0.0f?a/b:0.0f);
                    case OpCode::Mod: return AstValue::MakeFloat(std::fmod(a,b));
                    default: return AstValue::MakeFloat(0.0f);
                }
            }

            const int ia=lhs.ToInt();
            const int ib=rhs.ToInt();
            switch(op)
            {
                case OpCode::Add: return AstValue::MakeInt(ia+ib);
                case OpCode::Sub: return AstValue::MakeInt(ia-ib);
                case OpCode::Mul: return AstValue::MakeInt(ia*ib);
                case OpCode::Div: return AstValue::MakeInt(ib?ia/ib:0);
                case OpCode::Mod: return AstValue::MakeInt(ib?ia%ib:0);
                default: return AstValue::MakeInt(0);
            }
        }

        AstValue CompareValues(const AstValue &lhs,const AstValue &rhs,OpCode op)
        {
            if(lhs.type==TokenType::String || rhs.type==TokenType::String)
            {
                const std::string a=lhs.ToString();
                const std::string b=rhs.ToString();
                switch(op)
                {
                    case OpCode::Eq: return AstValue::MakeBool(a==b);
                    case OpCode::Ne: return AstValue::MakeBool(a!=b);
                    case OpCode::Lt: return AstValue::MakeBool(a<b);
                    case OpCode::Le: return AstValue::MakeBool(a<=b);
                    case OpCode::Gt: return AstValue::MakeBool(a>b);
                    case OpCode::Ge: return AstValue::MakeBool(a>=b);
                    default: return AstValue::MakeBool(false);
                }
            }

            const bool use_float=(lhs.type==TokenType::Float || rhs.type==TokenType::Float);
            if(use_float)
            {
                const float a=lhs.ToFloat();
                const float b=rhs.ToFloat();
                switch(op)
                {
                    case OpCode::Eq: return AstValue::MakeBool(a==b);
                    case OpCode::Ne: return AstValue::MakeBool(a!=b);
                    case OpCode::Lt: return AstValue::MakeBool(a<b);
                    case OpCode::Le: return AstValue::MakeBool(a<=b);
                    case OpCode::Gt: return AstValue::MakeBool(a>b);
                    case OpCode::Ge: return AstValue::MakeBool(a>=b);
                    default: return AstValue::MakeBool(false);
                }
            }

            const int ia=lhs.ToInt();
            const int ib=rhs.ToInt();
            switch(op)
            {
                case OpCode::Eq: return AstValue::MakeBool(ia==ib);
                case OpCode::Ne: return AstValue::MakeBool(ia!=ib);
                case OpCode::Lt: return AstValue::MakeBool(ia<ib);
                case OpCode::Le: return AstValue::MakeBool(ia<=ib);
                case OpCode::Gt: return AstValue::MakeBool(ia>ib);
                case OpCode::Ge: return AstValue::MakeBool(ia>=ib);
                default: return AstValue::MakeBool(false);
            }
        }
    }

    bool BytecodeModule::AddFunction(BytecodeFunction func)
    {
        if(func.name.empty())
            return false;

        if(functions.find(func.name)!=functions.end())
            return false;

        functions.emplace(func.name,std::move(func));
        return true;
    }

    BytecodeFunction *BytecodeModule::GetFunction(const std::string &name)
    {
        const auto it=functions.find(name);
        if(it==functions.end())
            return nullptr;
        return &it->second;
    }

    bool BytecodeVM::Execute(const std::string &func_name,const std::vector<AstValue> &args)
    {
        error.clear();
        last_result=AstValue::MakeVoid();
        value_stack.clear();
        callstack.clear();

        if(!module)
        {
            error="bytecode module not set";
            return false;
        }

        BytecodeFunction *func=module->GetFunction(func_name);
        if(!func)
        {
            error="bytecode function not found: "+func_name;
            return false;
        }

        return Execute(*func,args);
    }

    bool BytecodeVM::Execute(const BytecodeFunction &func,const std::vector<AstValue> &args)
    {
        error.clear();
        last_result=AstValue::MakeVoid();
        value_stack.clear();
        callstack.clear();

        if(static_cast<int32_t>(args.size())!=func.param_count)
        {
            error="bytecode param count mismatch";
            return false;
        }

        const size_t base=value_stack.size();
        for(const AstValue &v:args)
            value_stack.push_back(v);

        if(func.local_count>func.param_count)
        {
            const size_t extra=static_cast<size_t>(func.local_count-func.param_count);
            for(size_t i=0;i<extra;++i)
                value_stack.push_back(AstValue::MakeVoid());
        }

        callstack.push_back(Frame{&func,0,base});

        while(!callstack.empty())
        {
            if(!Step())
                break;
        }

        return error.empty();
    }

    bool BytecodeVM::Step()
    {
        if(callstack.empty())
            return false;

        Frame &frame=callstack.back();
        if(!frame.func)
        {
            error="bytecode missing frame function";
            return false;
        }

        if(frame.pc>=frame.func->code.size())
        {
            error="bytecode pc out of range";
            return false;
        }

        const Instruction ins=frame.func->code[frame.pc++];

        switch(ins.op)
        {
            case OpCode::Nop:
                return true;

            case OpCode::PushConst:
            {
                if(ins.a<0 || static_cast<size_t>(ins.a)>=frame.func->constants.size())
                {
                    error="bytecode constant out of range";
                    return false;
                }
                value_stack.push_back(frame.func->constants[static_cast<size_t>(ins.a)]);
                return true;
            }

            case OpCode::Pop:
            {
                bool ok=false;
                PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode pop underflow";
                    return false;
                }
                return true;
            }

            case OpCode::LoadLocal:
            {
                const size_t index=frame.base+static_cast<size_t>(ins.a);
                if(index>=value_stack.size())
                {
                    error="bytecode load local out of range";
                    return false;
                }
                value_stack.push_back(value_stack[index]);
                return true;
            }

            case OpCode::StoreLocal:
            {
                const size_t index=frame.base+static_cast<size_t>(ins.a);
                if(index>=value_stack.size())
                {
                    error="bytecode store local out of range";
                    return false;
                }
                bool ok=false;
                AstValue v=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode store local underflow";
                    return false;
                }
                value_stack[index]=std::move(v);
                return true;
            }

            case OpCode::Add:
            case OpCode::Sub:
            case OpCode::Mul:
            case OpCode::Div:
            case OpCode::Mod:
            {
                bool ok=false;
                AstValue rhs=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode binary op underflow";
                    return false;
                }
                AstValue lhs=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode binary op underflow";
                    return false;
                }

                if(ins.op==OpCode::Add && (lhs.type==TokenType::String || rhs.type==TokenType::String))
                {
                    value_stack.push_back(AstValue::MakeString(lhs.ToString()+rhs.ToString()));
                    return true;
                }

                value_stack.push_back(BinaryNumeric(lhs,rhs,ins.op));
                return true;
            }

            case OpCode::Neg:
            {
                bool ok=false;
                AstValue v=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode neg underflow";
                    return false;
                }
                if(v.type==TokenType::Float)
                    value_stack.push_back(AstValue::MakeFloat(-v.ToFloat()));
                else
                    value_stack.push_back(AstValue::MakeInt(-v.ToInt()));
                return true;
            }

            case OpCode::Not:
            {
                bool ok=false;
                AstValue v=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode not underflow";
                    return false;
                }
                value_stack.push_back(AstValue::MakeBool(!v.ToBool()));
                return true;
            }

            case OpCode::BitNot:
            {
                bool ok=false;
                AstValue v=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode bitnot underflow";
                    return false;
                }
                value_stack.push_back(AstValue::MakeInt(~v.ToInt()));
                return true;
            }

            case OpCode::BitAnd:
            case OpCode::BitOr:
            case OpCode::BitXor:
            case OpCode::Shl:
            case OpCode::Shr:
            {
                bool ok=false;
                AstValue rhs=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode bit op underflow";
                    return false;
                }
                AstValue lhs=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode bit op underflow";
                    return false;
                }
                const int a=lhs.ToInt();
                const int b=rhs.ToInt();
                switch(ins.op)
                {
                    case OpCode::BitAnd: value_stack.push_back(AstValue::MakeInt(a & b)); break;
                    case OpCode::BitOr: value_stack.push_back(AstValue::MakeInt(a | b)); break;
                    case OpCode::BitXor: value_stack.push_back(AstValue::MakeInt(a ^ b)); break;
                    case OpCode::Shl: value_stack.push_back(AstValue::MakeInt(a << b)); break;
                    case OpCode::Shr: value_stack.push_back(AstValue::MakeInt(a >> b)); break;
                    default: break;
                }
                return true;
            }

            case OpCode::Eq:
            case OpCode::Ne:
            case OpCode::Lt:
            case OpCode::Le:
            case OpCode::Gt:
            case OpCode::Ge:
            {
                bool ok=false;
                AstValue rhs=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode compare underflow";
                    return false;
                }
                AstValue lhs=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode compare underflow";
                    return false;
                }
                value_stack.push_back(CompareValues(lhs,rhs,ins.op));
                return true;
            }

            case OpCode::And:
            case OpCode::Or:
            {
                bool ok=false;
                AstValue rhs=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode logical op underflow";
                    return false;
                }
                AstValue lhs=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode logical op underflow";
                    return false;
                }
                const bool a=lhs.ToBool();
                const bool b=rhs.ToBool();
                value_stack.push_back(AstValue::MakeBool(ins.op==OpCode::And ? (a && b) : (a || b)));
                return true;
            }

            case OpCode::Jump:
            {
                if(ins.a<0 || static_cast<size_t>(ins.a)>=frame.func->code.size())
                {
                    error="bytecode jump out of range";
                    return false;
                }
                frame.pc=static_cast<size_t>(ins.a);
                return true;
            }

            case OpCode::JumpIfFalse:
            {
                bool ok=false;
                AstValue cond=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode jump if false underflow";
                    return false;
                }
                if(!cond.ToBool())
                {
                    if(ins.a<0 || static_cast<size_t>(ins.a)>=frame.func->code.size())
                    {
                        error="bytecode jump if false out of range";
                        return false;
                    }
                    frame.pc=static_cast<size_t>(ins.a);
                }
                return true;
            }

            case OpCode::CallNative:
            {
                if(!module || !module->GetHostModule())
                {
                    error="bytecode host module not set";
                    return false;
                }
                if(ins.a<0 || static_cast<size_t>(ins.a)>=frame.func->constants.size())
                {
                    error="bytecode call native constant out of range";
                    return false;
                }
                const AstValue &name_value=frame.func->constants[static_cast<size_t>(ins.a)];
                const std::string name=name_value.ToString();
                FuncMap *map=module->GetHostModule()->GetFuncMap(name);
                if(!map)
                {
                    error="bytecode native function not found: "+name;
                    return false;
                }

                const int argc=ins.b;
                if(argc<0 || static_cast<size_t>(argc)>value_stack.size())
                {
                    error="bytecode call native arg count";
                    return false;
                }

                std::vector<AstValue> args(static_cast<size_t>(argc));
                for(int i=argc-1;i>=0;--i)
                {
                    bool ok=false;
                    args[static_cast<size_t>(i)]=PopValue(value_stack,ok);
                    if(!ok)
                    {
                        error="bytecode call native arg underflow";
                        return false;
                    }
                }

                AstValue out;
                if(!CallNativeFunc(map,args,*module->GetHostModule(),out))
                {
                    error="bytecode call native failed: "+name;
                    return false;
                }

                value_stack.push_back(std::move(out));
                return true;
            }

            case OpCode::CallFunc:
            {
                if(!module)
                {
                    error="bytecode module not set";
                    return false;
                }
                if(ins.a<0 || static_cast<size_t>(ins.a)>=frame.func->constants.size())
                {
                    error="bytecode call func constant out of range";
                    return false;
                }
                const AstValue &name_value=frame.func->constants[static_cast<size_t>(ins.a)];
                const std::string name=name_value.ToString();
                BytecodeFunction *callee=module->GetFunction(name);
                if(!callee)
                {
                    error="bytecode function not found: "+name;
                    return false;
                }

                const int argc=ins.b;
                if(argc<0 || static_cast<size_t>(argc)>value_stack.size())
                {
                    error="bytecode call func arg count";
                    return false;
                }

                const size_t base=value_stack.size()-static_cast<size_t>(argc);
                if(callee->local_count>callee->param_count)
                {
                    const size_t extra=static_cast<size_t>(callee->local_count-callee->param_count);
                    for(size_t i=0;i<extra;++i)
                        value_stack.push_back(AstValue::MakeVoid());
                }

                callstack.push_back(Frame{callee,0,base});
                return true;
            }

            case OpCode::Ret:
            {
                bool ok=false;
                AstValue ret=PopValue(value_stack,ok);
                if(!ok)
                {
                    error="bytecode return underflow";
                    return false;
                }

                const size_t base=frame.base;
                callstack.pop_back();
                if(value_stack.size()>base)
                    value_stack.resize(base);

                if(callstack.empty())
                {
                    last_result=std::move(ret);
                    return false;
                }

                value_stack.push_back(std::move(ret));
                return true;
            }
        }

        error="bytecode unknown opcode";
        return false;
    }

    bool BytecodeVM::SaveState(std::vector<uint8_t> &) const
    {
        return false;
    }

    bool BytecodeVM::LoadState(const std::vector<uint8_t> &)
    {
        error="bytecode state load not implemented";
        return false;
    }
}
