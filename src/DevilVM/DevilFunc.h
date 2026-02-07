#pragma once

#include"DevilCommand.h"
#include<hgl/type/String.h>
#include<hgl/log/Log.h>
#include <absl/container/inlined_vector.h>
#include <memory>
#include <ankerl/unordered_dense.h>

namespace hgl
{
namespace devil
{
    class Module;

    /**
    * 虚拟机内脚本函数定义
    */
    class Func
    {
        OBJECT_LOGGER

        Module *module;

    public:

        U16String func_name;

        absl::InlinedVector<std::unique_ptr<Command>, 8> command;

        ankerl::unordered_dense::map<U16String,int> goto_flag;

        ankerl::unordered_dense::map<U16String,ValueInterface *> script_value_list;

    public:

        Func(Module *dvm,const U16String &name){module=dvm;func_name=name;}

        bool AddGotoFlag(const U16String &);      //增加跳转旗标
        int FindGotoFlag(const U16String &);      //查找跳转旗标

        void AddGotoCommand(const U16String &);   //增加跳转指令
        void AddReturn();                           //增加返回指令

        int AddCommand(Command *cmd)           //直接增加指令
        {
            command.emplace_back(cmd);
            return static_cast<int>(command.size()-1);
        }

        void AddScriptFuncCall(Func *);        //增加脚本函数呼叫

        ValueInterface *AddValue(eTokenType,const U16String &);          //增加一个变量
    };//class Func
}//namespace devil
}//namespace hgl
