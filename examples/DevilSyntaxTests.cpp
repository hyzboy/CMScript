#include <hgl/devil/DevilModule.h>
#include <hgl/devil/DevilContext.h>
#include <iostream>

using namespace hgl::devil;

int main(int,char **)
{
    const char *script=R"(
func main()
{
    int a=1;
    int b=2;
    int c=0;
    float f=1.5;
    bool ok=true;
    string s="hi";

    if(a<b && ok){ c=a+b; } else { c=a-b; }
    if((a+b)*2>3 || !ok){ c=c+1; }
    c = c + (a*b) - (a/b) + (a%b);
    c = c << 1; c = c >> 1; c = c & 3; c = c | 4; c = c ^ 5;

    do { c=c+1; } while(c<5);
    for(int i=0; i<3; i=i+1){ c=c+i; }
    switch(c){ case 1: c=10; break; case 2: c=20; break; default: c=30; }

    L1:; if(c>0) goto L2; goto END;
    L2: c=c+1; END:;
    c = helper(c);
    return c;
}

func helper(int x)
{
    return x+1;
}
)";

    Module module;
    module.SetUseBytecode(false);
    if(!module.AddScript(script,-1))
    {
        std::cout<<"syntax test: AddScript failed"<<std::endl;
        return -1;
    }

    Context ctx(&module);
    ctx.SetUseBytecode(false);
    ctx.Start("main");

    std::cout<<"syntax test: parsed and executed"<<std::endl;
    return 0;
}
