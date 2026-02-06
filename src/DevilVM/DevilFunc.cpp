#include"DevilFunc.h"
#include"DevilModule.h"

namespace hgl
{
    bool DevilFunc::AddGotoFlag(const U16String &name)
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

    int DevilFunc::FindGotoFlag(const U16String &name)
    {
        int index;

        if(goto_flag.Get(name,index))
            return(index);

        return -1;
    }

    void DevilFunc::AddGotoCommand(const U16String &name)
    {
        #ifdef _DEBUG
        const int index=command.Add(new DevilGoto(module,this,name));

        LogInfo(U16_TEXT("%s"),
            (U16String::numberOf(index)+U16_TEXT("\tgoto ")+name+U16_TEXT(";"))
                .c_str());
        #else
        command.Add(new DevilGoto(module,this,name));
        #endif//_DEBUG
    }

    void DevilFunc::AddReturn()
    {
        #ifdef _DEBUG
        const int index=command.Add(new DevilReturn(module));

        LogInfo(U16_TEXT("%s"),(U16String::numberOf(index)+U16_TEXT("\treturn;"))
                .c_str());
        #else
        command.Add(new DevilReturn(module));
        #endif//_DEBUG
    }

    void DevilFunc::AddScriptFuncCall(DevilFunc *script_func)
    {
        #ifdef _DEBUG
        const int index=command.Add(new DevilScriptFuncCall(module,script_func));

        LogInfo(U16_TEXT("%s"),
            (U16String::numberOf(index)+U16_TEXT("\t call ")+script_func->func_name)
                .c_str());
        #else
        command.Add(new DevilScriptFuncCall(module,script_func));
        #endif//
    }

    DevilValueInterface *DevilFunc::AddValue(eTokenType type,const U16String &name)
    {
        if(script_value_list.ContainsKey(name))
        {
            LogError(U16_TEXT("%s"),(U16_TEXT("添加变量失败，变量名称重复:")+name).c_str());

            return(nullptr);
        }

        if(type==ttBool     )return(new DevilScriptValue<bool       >(module,func_name,name,type));else
        if(type==ttString   )return(new DevilScriptValue<U16String>(module,func_name,name,type));else
        if(type==ttInt      )return(new DevilScriptValue<int        >(module,func_name,name,type));else
        if(type==ttUInt     )return(new DevilScriptValue<uint       >(module,func_name,name,type));else
        if(type==ttInt8     )return(new DevilScriptValue<int8       >(module,func_name,name,type));else
        if(type==ttUInt8    )return(new DevilScriptValue<uint8      >(module,func_name,name,type));else
        if(type==ttInt16    )return(new DevilScriptValue<int16      >(module,func_name,name,type));else
        if(type==ttUInt16   )return(new DevilScriptValue<uint16     >(module,func_name,name,type));else
        if(type==ttInt64    )return(new DevilScriptValue<int64      >(module,func_name,name,type));else
        if(type==ttUInt64   )return(new DevilScriptValue<uint64     >(module,func_name,name,type));else
        if(type==ttFloat    )return(new DevilScriptValue<float      >(module,func_name,name,type));else
        if(type==ttDouble   )return(new DevilScriptValue<double     >(module,func_name,name,type));else
        {
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("变量类型无法识别,name=")+name+U16_TEXT(",id=")
                      +U16String::numberOf(type)).c_str());
            return(nullptr);
        }
    }
}//namespace hgl

