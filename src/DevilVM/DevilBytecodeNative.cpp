#include <hgl/devil/DevilBytecode.h>
#include <hgl/devil/DevilModule.h>
#include "DevilCommand.h"

namespace hgl::devil
{
    namespace
    {
        bool SetParamFromValue(SystemFuncParam &out_param,TokenType expected,const AstValue &value,Module &module)
        {
            switch(expected)
            {
                case TokenType::Bool:
                    out_param.c=static_cast<char>(value.ToBool());
                    return true;
                case TokenType::Int:
                case TokenType::Int8:
                case TokenType::Int16:
                    out_param.i=value.ToInt();
                    return true;
                case TokenType::UInt:
                case TokenType::UInt8:
                case TokenType::UInt16:
                    out_param.u=value.ToUInt();
                    return true;
                case TokenType::Float:
                    out_param.f=value.ToFloat();
                    return true;
                case TokenType::Double:
                    out_param.d=value.ToDouble();
                    return true;
                case TokenType::String:
                {
                    std::string str=value.ToString();
                    module.string_list.push_back(str);
                    out_param.str=const_cast<char *>(module.string_list.back().c_str());
                    return true;
                }
                default:
                    return false;
            }
        }
    }

    bool CallNativeFunc(FuncMap *map,const std::vector<AstValue> &args,Module &module,AstValue &out_value)
    {
        const size_t param_count=map->param.size();
        if(args.size()!=param_count)
            return false;

        const size_t has_base=map->base?1u:0u;
        const size_t total_count=param_count+has_base;
        std::vector<SystemFuncParam> params(total_count);

        size_t offset=0;
        if(map->base)
        {
            params[0].void_pointer=map->base;
            offset=1;
        }

        for(size_t i=0;i<param_count;++i)
        {
            if(!SetParamFromValue(params[i+offset],map->param[i],args[i],module))
                return false;
        }

        const int param_size=static_cast<int>(total_count*sizeof(SystemFuncParam));

        if(map->result==TokenType::Void)
        {
            void *ret=nullptr;
            map->Call(params.data(),param_size,&ret);
            out_value=AstValue::MakeVoid();
            return true;
        }

        if(map->result==TokenType::Bool)
        {
            bool ret=false;
            map->Call(params.data(),param_size,&ret);
            out_value=AstValue::MakeBool(ret);
            return true;
        }

        if(map->result==TokenType::Int || map->result==TokenType::Int8 || map->result==TokenType::Int16)
        {
            int ret=0;
            map->Call(params.data(),param_size,&ret);
            out_value=AstValue::MakeInt(ret);
            return true;
        }

        if(map->result==TokenType::UInt || map->result==TokenType::UInt8 || map->result==TokenType::UInt16)
        {
            uint ret=0;
            map->Call(params.data(),param_size,&ret);
            out_value=AstValue::MakeUInt(ret);
            return true;
        }

        if(map->result==TokenType::Float)
        {
            float ret=0.0f;
            map->Call(params.data(),param_size,&ret);
            out_value=AstValue::MakeFloat(ret);
            return true;
        }

        if(map->result==TokenType::Double)
        {
            double ret=0.0;
            map->Call(params.data(),param_size,&ret);
            out_value=AstValue::MakeDouble(ret);
            return true;
        }

        if(map->result==TokenType::String)
        {
            char *ret=nullptr;
            map->Call(params.data(),param_size,&ret);
            out_value=AstValue::MakeString(ret?ret:"");
            return true;
        }

        return false;
    }
}
