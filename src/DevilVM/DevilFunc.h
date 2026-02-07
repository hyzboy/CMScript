#pragma once

#include "DevilAst.h"
#include <string>
#include <hgl/log/Log.h>
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

        std::unique_ptr<BlockStmt> body;
        ankerl::unordered_dense::map<std::string,size_t> label_index;

    public:

        Func(Module *dvm,const std::string &name){module=dvm;func_name=name;}

        void SetBody(std::unique_ptr<BlockStmt> new_body,ankerl::unordered_dense::map<std::string,size_t> labels)
        {
            body=std::move(new_body);
            label_index=std::move(labels);
        }

        const BlockStmt *GetBody() const{return body.get();}
        const ankerl::unordered_dense::map<std::string,size_t> &GetLabels() const{return label_index;}
    };//class Func
}//namespace hgl::devil
