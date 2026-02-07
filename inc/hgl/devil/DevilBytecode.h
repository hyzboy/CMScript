#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <ankerl/unordered_dense.h>
#include <hgl/devil/DevilValue.h>

namespace hgl::devil
{
    class Module;

    enum class OpCode : uint8_t
    {
        Nop,
        PushConst,
        Pop,
        LoadLocal,
        StoreLocal,
        Cast,
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Neg,
        Not,
        BitNot,
        BitAnd,
        BitOr,
        BitXor,
        Shl,
        Shr,
        Eq,
        Ne,
        Lt,
        Le,
        Gt,
        Ge,
        And,
        Or,
        Jump,
        JumpIfFalse,
        CallNative,
        CallFunc,
        Ret
    };

    struct Instruction
    {
        OpCode op=OpCode::Nop;
        int32_t a=0;
        int32_t b=0;
        int32_t c=0;
    };

    struct BytecodeFunction
    {
        std::string name;
        std::vector<Instruction> code;
        std::vector<AstValue> constants;
        std::vector<int32_t> const_ints;
        int32_t local_count=0;
        int32_t param_count=0;
        bool fast_int=false;
    };

    class BytecodeModule
    {
        Module *host_module=nullptr;
        ankerl::unordered_dense::map<std::string,BytecodeFunction> functions;

    public:
        void SetHostModule(Module *module){host_module=module;}
        Module *GetHostModule() const{return host_module;}

        bool AddFunction(BytecodeFunction func);
        BytecodeFunction *GetFunction(const std::string &name);
        void Clear();
        const ankerl::unordered_dense::map<std::string,BytecodeFunction> &GetFunctions() const{return functions;}

        bool Serialize(std::vector<uint8_t> &out_data) const;
        bool Deserialize(const std::vector<uint8_t> &data);
    };

    struct Frame
    {
        const BytecodeFunction *func=nullptr;
        size_t pc=0;
        size_t base=0;
    };

    class BytecodeVM
    {
        BytecodeModule *module=nullptr;
        std::vector<AstValue> value_stack;
        std::vector<Frame> callstack;
        std::vector<AstValue> arg_buffer;
        AstValue last_result=AstValue::MakeVoid();
        std::string error;

    public:
        explicit BytecodeVM(BytecodeModule *m=nullptr):module(m){}

        void SetModule(BytecodeModule *m){module=m;}
        const std::string &GetError() const{return error;}
        const AstValue &GetLastResult() const{return last_result;}

        bool Execute(const std::string &func_name,const std::vector<AstValue> &args);
        bool Execute(const BytecodeFunction &func,const std::vector<AstValue> &args);
        bool Step();

        bool SaveState(std::vector<uint8_t> &) const;
        bool LoadState(const std::vector<uint8_t> &);
    };

    const char *OpCodeToString(OpCode op);
    std::string DisassembleInstruction(const BytecodeFunction &func,size_t index);
    std::string DisassembleFunction(const BytecodeFunction &func);
    std::string DisassembleModule(const BytecodeModule &module);
}
