#ifndef DevilVariableH
#define DevilVariableH

#include<hgl/type/String.h>
namespace hgl
{
namespace devil
{
    class Engine;

    class Variable
    {
        Engine *vm;

    public:

        U16String var_name;

    public:

        Variable(Engine *dvm,const U16String &name){vm=dvm;var_name=name;}
    };//class Variable
}//namespace devil
}//namespace hgl
#endif//DevilVariableH
