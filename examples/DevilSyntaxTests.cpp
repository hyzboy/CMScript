#include <hgl/devil/DevilModule.h>
#include <hgl/devil/DevilContext.h>
#include <iostream>

using namespace hgl::devil;

namespace
{
    void PrintInt(int v)
    {
        std::cout<<"[script] int="<<v<<std::endl;
    }
}

int main(int,char **)
{
    const char *script=R"(
int main()
{
    enum Color : int { Red, Green=3, Blue };
    int e=Color::Blue;
    print_int(e);

    int a=1;
    int b=2;
    int c=0;
    float f=1.5;
    bool ok=true;
    string s="hi";

    if(a<b && ok){ c=a+b; } else { c=a-b; }
    print_int(c);
    if((a+b)*2>3 || !ok){ c=c+1; }
    print_int(c);
    c = c + (a*b) - (a/b) + (a%b);
    print_int(c);
    c = c << 1; c = c >> 1; c = c & 3; c = c | 4; c = c ^ 5;
    print_int(c);

    do { c=c+1; } while(c<5);
    print_int(c);
    for(int i=0; i<3; i=i+1){ c=c+i; }
    print_int(c);
    switch(e){ case Color::Red: c=10; break; case Color::Green: c=20; break; default: c=30; }
    print_int(c);
    switch(c){ case 1: c=10; break; case 2: c=20; break; default: c=30; }
    print_int(c);

    L1:; if(c>0) goto L2; goto END;
    L2: c=c+1; END:;
    print_int(c);
    c = helper(c);
    print_int(c);
    return c;
}

int helper(int x)
{
    return x+1;
}
)";

    Module module;
    module.MapFunc("print_int",&PrintInt);
    module.SetUseBytecode(false);
    if(!module.AddScript(script,-1))
    {
        std::cout<<"syntax test: AddScript failed"<<std::endl;
        return -1;
    }

    Context ctx(&module);
    ctx.SetUseBytecode(false);
    const AstValue result=ctx.Call("main");
    std::cout<<"syntax test: main returned "<<result.ToString()<<std::endl;
    return 0;
}
