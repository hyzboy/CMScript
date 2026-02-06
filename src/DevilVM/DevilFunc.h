#pragma once

#include"DevilCommand.h"
#include<hgl/type/String.h>
#include<hgl/log/Log.h>
#include<hgl/type/ManagedArray.h>
#include<hgl/type/UnorderedMap.h>

namespace hgl
{
    class DevilScriptModule;

    /**
    * 虚拟机内脚本函数定义
    */
    class DevilFunc
    {
        OBJECT_LOGGER

        DevilScriptModule *module;

    public:

        U16String func_name;

        ManagedArray<DevilCommand> command;

        UnorderedMap<U16String,int> goto_flag;

        UnorderedMap<U16String,DevilValueInterface *> script_value_list;

    public:

        DevilFunc(DevilScriptModule *dvm,const U16String &name){module=dvm;func_name=name;}

        bool AddGotoFlag(const U16String &);      //增加跳转旗标
        int FindGotoFlag(const U16String &);      //查找跳转旗标

        void AddGotoCommand(const U16String &);   //增加跳转指令
        void AddReturn();                           //增加返回指令

        int AddCommand(DevilCommand *cmd)           //直接增加指令
        {
            return command.Add(cmd);
        }

        void AddScriptFuncCall(DevilFunc *);        //增加脚本函数呼叫

        DevilValueInterface *AddValue(eTokenType,const U16String &);          //增加一个变量
    };//class DevilFunc
}//namespace hgl
