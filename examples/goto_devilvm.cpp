#include <iostream>

#include <hgl/type/String.h>

#include <hgl/devil/DevilVM.h>

namespace
{
    int g_value = 0;

    void SetValue(int v)
    {
        g_value = v;

        std::cout<<"SetValue called with "<<v<<"."<<std::endl;
    }
}

int main()
{
    hgl::devil::Module module;

    if(!module.MapFunc(U16_TEXT("void set(int)"), (void *)(&SetValue)))
    {
        std::cerr << "MapFunc failed." << std::endl;
        return 1;
    }

    const u16char *script = U16_TEXT(
        "func main(){"
        " goto L2;"
        " L1: set(1); goto END;"
        " L2: set(2); goto L1;"
        " END:;"
        " }"
    );

    if(!module.AddScript(script))
    {
        std::cerr << "AddScript failed." << std::endl;
        return 1;
    }

    hgl::devil::Context context(&module);

    if(!context.Start(U16_TEXT("main")))
    {
        std::cerr << "Script execution failed." << std::endl;
        return 1;
    }

    if(g_value != 1)
    {
        std::cerr << "Goto test failed. Expected 1, got " << g_value << "." << std::endl;
        return 1;
    }

    std::cout << "Goto test passed." << std::endl;
    return 0;
}
