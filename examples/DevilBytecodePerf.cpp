#include <hgl/devil/DevilModule.h>
#include <hgl/devil/DevilContext.h>
#include <hgl/devil/DevilBytecode.h>
#include "DevilBytecodeBuilder.h"
#include "DevilFunc.h"
#include <iostream>
#include <chrono>

using namespace hgl::devil;

namespace
{
    double ElapsedMs(const std::chrono::steady_clock::time_point &start,const std::chrono::steady_clock::time_point &end)
    {
        return std::chrono::duration<double,std::milli>(end-start).count();
    }
}

int main(int,char **)
{
    const char *script=
        "func main() { "
        "int a=0; int i=0; "
        "while(i<100000){ a=a+1; i=i+1; } "
        "return a; "
        "}";

    Module module;
    Context ctx(&module);

    module.SetUseBytecode(false);
    if(!module.AddScript(script,-1))
    {
        std::cout<<"perf test: AddScript failed"<<std::endl;
        return -1;
    }

    ctx.SetUseBytecode(false);
    auto start_ast=std::chrono::steady_clock::now();
    ctx.Start("main");
    auto end_ast=std::chrono::steady_clock::now();

    std::vector<uint8_t> bc_data;
    module.SetUseBytecode(true);
    if(!module.BuildBytecode())
    {
        std::cout<<"perf test: BuildBytecode failed"<<std::endl;
        return -1;
    }

    BytecodeModule *bc_module=module.GetBytecodeModule();
    if(!bc_module->Serialize(bc_data))
    {
        std::cout<<"perf test: Serialize failed"<<std::endl;
        return -1;
    }

    bc_module->Clear();
    if(!bc_module->Deserialize(bc_data))
    {
        std::cout<<"perf test: Deserialize failed"<<std::endl;
        return -1;
    }
    bc_module->SetHostModule(&module);

    ctx.SetUseBytecode(true);
    auto start_bc=std::chrono::steady_clock::now();
    ctx.Start("main");
    auto end_bc=std::chrono::steady_clock::now();

    std::cout<<"AST exec ms: "<<ElapsedMs(start_ast,end_ast)<<"\n";
    std::cout<<"Bytecode exec ms: "<<ElapsedMs(start_bc,end_bc)<<"\n";
    return 0;
}
