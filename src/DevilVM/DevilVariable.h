#pragma once

#include <string>

namespace hgl::devil
{
    class Engine;

    class Variable
    {
        Engine *vm;

    public:

        std::string var_name;

    public:

        Variable(Engine *dvm,const std::string &name){vm=dvm;var_name=name;}
    };//class Variable
}//namespace hgl::devil
