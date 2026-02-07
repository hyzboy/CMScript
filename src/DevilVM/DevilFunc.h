#pragma once

#include "DevilCommand.h"
#include <string>
#include <hgl/log/Log.h>
#include <absl/container/inlined_vector.h>
#include <memory>
#include <ankerl/unordered_dense.h>

namespace hgl::devil
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

        std::string func_name;

        absl::InlinedVector<std::unique_ptr<Command>, 8> command;

        ankerl::unordered_dense::map<std::string,int> goto_flag;

        ankerl::unordered_dense::map<std::string,ValueInterface *> script_value_list;

    public:

        Func(Module *dvm,const std::string &name){module=dvm;func_name=name;}

        bool AddGotoFlag(const std::string &);      //增加跳转旗标
        int FindGotoFlag(const std::string &);      //查找跳转旗标

        void AddGotoCommand(const std::string &);   //增加跳转指令
        void AddReturn();                           //增加返回指令

        int AddCommand(Command *cmd)           //直接增加指令
        {
            command.emplace_back(cmd);
            return static_cast<int>(command.size()-1);
        }

        void AddScriptFuncCall(Func *);        //增加脚本函数呼叫

        ValueInterface *AddValue(eTokenType,const std::string &);          //增加一个变量
    };//class Func
}//namespace hgl::devil
