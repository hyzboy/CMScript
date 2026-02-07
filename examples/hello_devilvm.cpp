#include <iostream>
#include <string>
#include <hgl/devil/DevilVM.h>

namespace
{
    void PrintU16(const char *text)
    {
        if(!text)
        {
            std::cout << "(null)" << std::endl;
            return;
        }

        std::cout << text << std::endl;
    }
}

int main()
{
    hgl::devil::Module module;

    if(!module.MapFunc("print", &PrintU16))
    {
        std::cerr << "MapFunc failed." << std::endl;
        return 1;
    }

    const char *script = "void main(){ print(\"hello,world!\"); }";

    if(!module.AddScript(script))
    {
        std::cerr << "AddScript failed." << std::endl;
        return 1;
    }

    hgl::devil::Context context(&module);

    if(!context.Start("main"))
    {
        std::cerr << "Script execution failed." << std::endl;
        return 1;
    }

    return 0;
}
