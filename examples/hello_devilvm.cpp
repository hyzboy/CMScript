#include <iostream>
#include <string>
#include <codecvt>
#include <locale>

#include <hgl/type/String.h>

#include <hgl/devil/DevilVM.h>

namespace
{
    void PrintU16(const u16char *text)
    {
        if(!text)
        {
            std::cout << "(null)" << std::endl;
            return;
        }

        std::u16string u16(reinterpret_cast<const char16_t *>(text));
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
        std::cout << conv.to_bytes(u16.c_str()) << std::endl;
    }
}

int main()
{
    hgl::devil::Module module;

    if(!module.MapFunc(U16_TEXT("void print(string)"), (void *)(&PrintU16)))
    {
        std::cerr << "MapFunc failed." << std::endl;
        return 1;
    }

    const u16char *script = U16_TEXT("func main(){ print(\"hello,world!\"); }");

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

    return 0;
}
