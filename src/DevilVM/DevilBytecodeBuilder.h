#pragma once

#include <string>
#include <vector>
#include <ankerl/unordered_dense.h>
#include <hgl/devil/DevilBytecode.h>
#include "DevilAst.h"
#include "DevilFunc.h"

namespace hgl::devil
{
    class Module;

    class BytecodeBuilder
    {
        Module *host_module=nullptr;
        BytecodeModule *bytecode_module=nullptr;
        std::string error;

        ankerl::unordered_dense::map<std::string,int32_t> locals;
        ankerl::unordered_dense::map<std::string,size_t> labels;

        struct PendingJump
        {
            size_t index=0;
            std::string label;
        };

        std::vector<PendingJump> pending_gotos;

        struct LoopContext
        {
            std::vector<size_t> breaks;
            std::vector<size_t> continues;
            size_t continue_target=0;
            bool allow_continue=true;
        };

        std::vector<LoopContext> loop_stack;

        int32_t AddLocal(const std::string &name);
        int32_t GetLocal(const std::string &name) const;

        int32_t AddConst(BytecodeFunction &func,AstValue value);
        size_t Emit(BytecodeFunction &func,OpCode op,int32_t a=0,int32_t b=0,int32_t c=0);

        bool BuildExpr(BytecodeFunction &func,const Expr *expr);
        bool BuildStmt(BytecodeFunction &func,const Stmt *stmt);
        bool BuildBlock(BytecodeFunction &func,const BlockStmt *block);

    public:
        BytecodeBuilder()=default;
        explicit BytecodeBuilder(Module *module,BytecodeModule *out)
            : host_module(module), bytecode_module(out){}

        void SetHostModule(Module *module){host_module=module;}
        void SetOutput(BytecodeModule *out){bytecode_module=out;}
        const std::string &GetError() const{return error;}

        bool BuildFunction(const Func *func,BytecodeFunction &out_func);
        bool BuildAndAdd(const Func *func);
    };
}
