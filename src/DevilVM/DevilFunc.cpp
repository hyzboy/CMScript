#include"DevilFunc.h"
#include <hgl/devil/DevilModule.h>

namespace hgl
{
namespace devil
{
    bool Func::AddGotoFlag(const std::string &name)
    {
        int count=static_cast<int>(command.size());

        if(goto_flag.find(name)==goto_flag.end())
        {
            goto_flag.emplace(name,count);

            LogInfo("%s",(":"+name).c_str());

            return(true);
        }
        else
        {
            LogInfo("%s",("添加跳转标识符失败，这个标识符重复了:"+name).c_str());

            return(false);
        }
    }

    int Func::FindGotoFlag(const std::string &name)
    {
        int index;

        const auto it=goto_flag.find(name);
        if(it!=goto_flag.end())
            return(it->second);

        return -1;
    }

    void Func::AddGotoCommand(const std::string &name)
    {
        #ifdef _DEBUG
        command.emplace_back(std::make_unique<Goto>(module,this,name));
        const int index=static_cast<int>(command.size()-1);

        LogInfo("%s",
            (std::to_string(index)+"\tgoto "+name+";")
                .c_str());
        #else
        command.emplace_back(std::make_unique<Goto>(module,this,name));
        #endif//_DEBUG
    }

    void Func::AddReturn()
    {
        #ifdef _DEBUG
        command.emplace_back(std::make_unique<Return>(module));
        const int index=static_cast<int>(command.size()-1);

        LogInfo("%s",(std::to_string(index)+"\treturn;")
            .c_str());
        #else
        command.emplace_back(std::make_unique<Return>(module));
        #endif//_DEBUG
    }

    void Func::AddScriptFuncCall(Func *script_func)
    {
        #ifdef _DEBUG
        command.emplace_back(std::make_unique<ScriptFuncCall>(module,script_func));
        const int index=static_cast<int>(command.size()-1);

        LogInfo("%s",
            (std::to_string(index)+"\t call "+script_func->func_name)
                .c_str());
        #else
        command.emplace_back(std::make_unique<ScriptFuncCall>(module,script_func));
        #endif//
    }

    ValueInterface *Func::AddValue(eTokenType type,const std::string &name)
    {
        if(script_value_list.find(name)!=script_value_list.end())
        {
            LogError("%s",("添加变量失败，变量名称重复:"+name).c_str());

            return(nullptr);
        }

        if(type==ttBool     )return(new ScriptValue<bool       >(module,func_name,name,type));else
        if(type==ttString   )return(new ScriptValue<std::string>(module,func_name,name,type));else
        if(type==ttInt      )return(new ScriptValue<int        >(module,func_name,name,type));else
        if(type==ttUInt     )return(new ScriptValue<uint       >(module,func_name,name,type));else
        if(type==ttInt8     )return(new ScriptValue<int8       >(module,func_name,name,type));else
        if(type==ttUInt8    )return(new ScriptValue<uint8      >(module,func_name,name,type));else
        if(type==ttInt16    )return(new ScriptValue<int16      >(module,func_name,name,type));else
        if(type==ttUInt16   )return(new ScriptValue<uint16     >(module,func_name,name,type));else
        if(type==ttInt64    )return(new ScriptValue<int64      >(module,func_name,name,type));else
        if(type==ttUInt64   )return(new ScriptValue<uint64     >(module,func_name,name,type));else
        if(type==ttFloat    )return(new ScriptValue<float      >(module,func_name,name,type));else
        if(type==ttDouble   )return(new ScriptValue<double     >(module,func_name,name,type));else
        {
            LogError("%s",
                     ("变量类型无法识别,name="+name+",id="
                      +std::to_string(type)).c_str());
            return(nullptr);
        }
    }
}//namespace devil
}//namespace hgl

