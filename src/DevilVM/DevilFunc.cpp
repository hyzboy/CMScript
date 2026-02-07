#include"DevilFunc.h"
#include"DevilModule.h"

namespace hgl
{
namespace devil
{
    bool Func::AddGotoFlag(const U16String &name)
    {
        int count=command.GetCount();

        if(!goto_flag.ContainsKey(name))
        {
            goto_flag.Add(name,count);

            LogInfo(U16_TEXT("%s"),(U16_TEXT(":")+name).c_str());

            return(true);
        }
        else
        {
            LogInfo(U16_TEXT("%s"),(U16_TEXT("添加跳转标识符失败，这个标识符重复了:")+name).c_str());

            return(false);
        }
    }

    int Func::FindGotoFlag(const U16String &name)
    {
        int index;

        if(goto_flag.Get(name,index))
            return(index);

        return -1;
    }

    void Func::AddGotoCommand(const U16String &name)
    {
        #ifdef _DEBUG
        const int index=command.Add(new Goto(module,this,name));

        LogInfo(U16_TEXT("%s"),
            (U16String::numberOf(index)+U16_TEXT("\tgoto ")+name+U16_TEXT(";"))
                .c_str());
        #else
        command.Add(new Goto(module,this,name));
        #endif//_DEBUG
    }

    void Func::AddReturn()
    {
        #ifdef _DEBUG
        const int index=command.Add(new Return(module));

        LogInfo(U16_TEXT("%s"),(U16String::numberOf(index)+U16_TEXT("\treturn;"))
                .c_str());
        #else
        command.Add(new Return(module));
        #endif//_DEBUG
    }

    void Func::AddScriptFuncCall(Func *script_func)
    {
        #ifdef _DEBUG
        const int index=command.Add(new ScriptFuncCall(module,script_func));

        LogInfo(U16_TEXT("%s"),
            (U16String::numberOf(index)+U16_TEXT("\t call ")+script_func->func_name)
                .c_str());
        #else
        command.Add(new ScriptFuncCall(module,script_func));
        #endif//
    }

    ValueInterface *Func::AddValue(eTokenType type,const U16String &name)
    {
        if(script_value_list.ContainsKey(name))
        {
            LogError(U16_TEXT("%s"),(U16_TEXT("添加变量失败，变量名称重复:")+name).c_str());

            return(nullptr);
        }

        if(type==ttBool     )return(new ScriptValue<bool       >(module,func_name,name,type));else
        if(type==ttString   )return(new ScriptValue<U16String>(module,func_name,name,type));else
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
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("变量类型无法识别,name=")+name+U16_TEXT(",id=")
                      +U16String::numberOf(type)).c_str());
            return(nullptr);
        }
    }
}//namespace devil
}//namespace hgl

