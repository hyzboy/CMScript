#ifndef DevilVariableH
#define DevilVariableH

#include<hgl/type/String.h>
namespace hgl
{
    class DevilEngine;

    class DevilVariable
    {
        DevilEngine *vm;

    public:

        U16String var_name;

    public:

        DevilVariable(DevilEngine *dvm,const U16String &name){vm=dvm;var_name=name;}
    };//class DevilVariable
}//namespace hgl
#endif//DevilVariableH
