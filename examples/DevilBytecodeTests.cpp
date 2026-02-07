#include <hgl/devil/DevilModule.h>
#include <hgl/devil/DevilBytecode.h>
#include "DevilBytecodeBuilder.h"
#include "DevilFunc.h"
#include <iostream>

using namespace hgl::devil;

int main(int,char **)
{
    Module module;
    BytecodeModule bc_module;
    BytecodeBuilder builder(&module,&bc_module);

    const char *script=
        "func main() { "
        "int a=1; int b=2; "
        "if(a<b){ b=b+1; } else { b=b-1; } "
        "return add(a,b); "
        "} "
        "func looptest() { "
        "int x=0; "
        "do { x=x+1; } while(x<3); "
        "switch(x){ case 3: x=7; break; default: x=9; } "
        "return x; "
        "} "
        "func add(int x,int y){ return x+y; }";

    if(!module.AddScript(script,-1))
    {
        std::cout<<"bytecode test: AddScript failed"<<std::endl;
        return -1;
    }

    bc_module.SetHostModule(&module);

    const char *funcs[]={"main","looptest","add"};
    for(const char *name:funcs)
    {
        Func *func=module.GetScriptFunc(name);
        if(!func)
        {
            std::cout<<"bytecode test: missing func "<<name<<std::endl;
            return -1;
        }

        if(!builder.BuildAndAdd(func))
        {
            std::cout<<"bytecode test: build failed "<<builder.GetError()<<std::endl;
            return -1;
        }
    }

    const std::string disasm=DisassembleModule(bc_module);
    if(disasm.empty())
    {
        std::cout<<"bytecode test: empty disassembly"<<std::endl;
        return -1;
    }

    std::cout<<disasm;

    BytecodeVM vm(&bc_module);
    if(!vm.Execute("main",{}))
    {
        std::cout<<"bytecode test: execute failed "<<vm.GetError()<<std::endl;
        return -1;
    }

    std::cout<<"bytecode test: main returned "<<vm.GetLastResult().ToString()<<std::endl;

    return 0;
}
