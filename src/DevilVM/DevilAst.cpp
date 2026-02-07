#include "DevilAst.h"
#include "DevilCommand.h"
#include <hgl/devil/DevilModule.h>
#include <hgl/devil/DevilContext.h>
#include <hgl/devil/DevilValue.h>
#include <cmath>

namespace hgl::devil
{
    namespace
    {
        AstValue MakeDefaultValue(TokenType type)
        {
            switch(type)
            {
                case TokenType::Bool:   return AstValue::MakeBool(false);
                case TokenType::Int:    return AstValue::MakeInt(0);
                case TokenType::Int8:   return AstValue::MakeInt(0);
                case TokenType::Int16:  return AstValue::MakeInt(0);
                case TokenType::UInt:   return AstValue::MakeUInt(0);
                case TokenType::UInt8:  return AstValue::MakeUInt(0);
                case TokenType::UInt16: return AstValue::MakeUInt(0);
                case TokenType::Float:  return AstValue::MakeFloat(0.0f);
                case TokenType::String: return AstValue::MakeString(std::string());
                case TokenType::Void:   return AstValue::MakeVoid();
                default:                 return AstValue::MakeVoid();
            }
        }

        AstValue CastValue(const AstValue &value,TokenType type)
        {
            switch(type)
            {
                case TokenType::Bool:   return AstValue::MakeBool(value.ToBool());
                case TokenType::Int:
                case TokenType::Int8:
                case TokenType::Int16:  return AstValue::MakeInt(value.ToInt());
                case TokenType::UInt:
                case TokenType::UInt8:
                case TokenType::UInt16: return AstValue::MakeUInt(value.ToUInt());
                case TokenType::Float:  return AstValue::MakeFloat(value.ToFloat());
                case TokenType::String: return AstValue::MakeString(value.ToString());
                default:                 return AstValue::MakeVoid();
            }
        }

        bool SetParamFromValue(SystemFuncParam &out_param,TokenType expected,const AstValue &value,ExecContext &ctx)
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
                case TokenType::String:
                {
                    std::string str=value.ToString();
                    ctx.module->string_list.push_back(str);
                    out_param.str=const_cast<char *>(ctx.module->string_list.back().c_str());
                    return true;
                }
                default:
                    return false;
            }
        }

        AstValue CallNative(FuncMap *map,const std::vector<AstValue> &args,ExecContext &ctx)
        {
            const size_t param_count=map->param.size();

            if(args.size()!=param_count)
            {
                ctx.error="native call param count mismatch";
                return AstValue::MakeVoid();
            }

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
                if(!SetParamFromValue(params[i+offset],map->param[i],args[i],ctx))
                {
                    ctx.error="native call param type mismatch";
                    return AstValue::MakeVoid();
                }
            }

            const int param_size=static_cast<int>(total_count*sizeof(SystemFuncParam));

            if(map->result==TokenType::Void)
            {
                void *ret=nullptr;
                map->Call(params.data(),param_size,&ret);
                return AstValue::MakeVoid();
            }

            if(map->result==TokenType::Bool)
            {
                bool ret=false;
                map->Call(params.data(),param_size,&ret);
                return AstValue::MakeBool(ret);
            }

            if(map->result==TokenType::Int || map->result==TokenType::Int8 || map->result==TokenType::Int16)
            {
                int ret=0;
                map->Call(params.data(),param_size,&ret);
                return AstValue::MakeInt(static_cast<int32_t>(ret));
            }

            if(map->result==TokenType::UInt || map->result==TokenType::UInt8 || map->result==TokenType::UInt16)
            {
                uint ret=0;
                map->Call(params.data(),param_size,&ret);
                return AstValue::MakeUInt(static_cast<uint32_t>(ret));
            }

            if(map->result==TokenType::Float)
            {
                float ret=0.0f;
                map->Call(params.data(),param_size,&ret);
                return AstValue::MakeFloat(ret);
            }

            if(map->result==TokenType::String)
            {
                char *ret=nullptr;
                map->Call(params.data(),param_size,&ret);
                if(ret)
                    return AstValue::MakeString(ret);
                return AstValue::MakeString(std::string());
            }

            ctx.error="native call return type unsupported";
            return AstValue::MakeVoid();
        }
    }

    AstValue AstValue::MakeVoid(){return AstValue{TokenType::Void,{}};}
    AstValue AstValue::MakeBool(bool v){return AstValue{TokenType::Bool,v};}
    AstValue AstValue::MakeInt(int32_t v){return AstValue{TokenType::Int,v};}
    AstValue AstValue::MakeUInt(uint32_t v){return AstValue{TokenType::UInt,v};}
    AstValue AstValue::MakeFloat(float v){return AstValue{TokenType::Float,v};}
    AstValue AstValue::MakeString(std::string v){return AstValue{TokenType::String,std::move(v)};}

    bool AstValue::IsNumeric() const
    {
        return type==TokenType::Bool || type==TokenType::Int || type==TokenType::UInt || type==TokenType::Float
            || type==TokenType::Int8 || type==TokenType::Int16 || type==TokenType::UInt8 || type==TokenType::UInt16;
    }

    bool AstValue::ToBool() const
    {
        if(type==TokenType::Bool)
            return std::get<bool>(data);
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return std::get<int32_t>(data)!=0;
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return std::get<uint32_t>(data)!=0u;
        if(type==TokenType::Float)
            return std::fabs(std::get<float>(data))>0.000001f;
        return false;
    }

    int32_t AstValue::ToInt() const
    {
        if(type==TokenType::Bool)
            return std::get<bool>(data)?1:0;
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return std::get<int32_t>(data);
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return static_cast<int32_t>(std::get<uint32_t>(data));
        if(type==TokenType::Float)
            return static_cast<int32_t>(std::get<float>(data));
        return 0;
    }

    uint32_t AstValue::ToUInt() const
    {
        if(type==TokenType::Bool)
            return std::get<bool>(data)?1u:0u;
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return static_cast<uint32_t>(std::get<int32_t>(data));
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return std::get<uint32_t>(data);
        if(type==TokenType::Float)
            return static_cast<uint32_t>(std::get<float>(data));
        return 0u;
    }

    float AstValue::ToFloat() const
    {
        if(type==TokenType::Bool)
            return std::get<bool>(data)?1.0f:0.0f;
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return static_cast<float>(std::get<int32_t>(data));
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return static_cast<float>(std::get<uint32_t>(data));
        if(type==TokenType::Float)
            return std::get<float>(data);
        return 0.0f;
    }

    std::string AstValue::ToString() const
    {
        if(type==TokenType::String)
            return std::get<std::string>(data);
        if(type==TokenType::Bool)
            return std::get<bool>(data)?"true":"false";
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return std::to_string(std::get<int32_t>(data));
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return std::to_string(std::get<uint32_t>(data));
        if(type==TokenType::Float)
            return std::to_string(std::get<float>(data));
        return std::string();
    }

    ExecResult ExecResult::Normal(){return ExecResult{ExecFlow::Normal,AstValue::MakeVoid(),{}, {}};}
    ExecResult ExecResult::Return(AstValue v){return ExecResult{ExecFlow::Return,std::move(v),{}, {}};}
    ExecResult ExecResult::Goto(const std::string &label){return ExecResult{ExecFlow::Goto,AstValue::MakeVoid(),label,{}};}
    ExecResult ExecResult::Error(const std::string &message){return ExecResult{ExecFlow::Error,AstValue::MakeVoid(),{},message};}

    AstValue IdentifierExpr::Eval(ExecContext &ctx) const
    {
        const auto it=ctx.locals.find(name);
        if(it==ctx.locals.end())
        {
            ctx.error="unknown identifier: "+name;
            return AstValue::MakeVoid();
        }
        return it->second;
    }

    AstValue UnaryExpr::Eval(ExecContext &ctx) const
    {
        AstValue val=expr->Eval(ctx);
        if(!ctx.error.empty())
            return AstValue::MakeVoid();

        switch(op)
        {
            case TokenType::Minus:
                if(!val.IsNumeric())
                {
                    ctx.error="unary '-' expects numeric";
                    return AstValue::MakeVoid();
                }
                return AstValue::MakeFloat(-val.ToFloat());
            case TokenType::Plus:
                return val;
            case TokenType::Not:
                return AstValue::MakeBool(!val.ToBool());
            default:
                ctx.error="unsupported unary operator";
                return AstValue::MakeVoid();
        }
    }

    AstValue BinaryExpr::Eval(ExecContext &ctx) const
    {
        AstValue lhs=left->Eval(ctx);
        if(!ctx.error.empty())
            return AstValue::MakeVoid();
        AstValue rhs=right->Eval(ctx);
        if(!ctx.error.empty())
            return AstValue::MakeVoid();

        switch(op)
        {
            case TokenType::Plus:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeFloat(lhs.ToFloat()+rhs.ToFloat());
                ctx.error="'+' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Minus:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeFloat(lhs.ToFloat()-rhs.ToFloat());
                ctx.error="'-' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Star:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeFloat(lhs.ToFloat()*rhs.ToFloat());
                ctx.error="'*' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Slash:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeFloat(lhs.ToFloat()/rhs.ToFloat());
                ctx.error="'/' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Equal:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeBool(lhs.ToFloat()==rhs.ToFloat());
                return AstValue::MakeBool(lhs.ToString()==rhs.ToString());
            case TokenType::NotEqual:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeBool(lhs.ToFloat()!=rhs.ToFloat());
                return AstValue::MakeBool(lhs.ToString()!=rhs.ToString());
            case TokenType::LessThan:
                return AstValue::MakeBool(lhs.ToFloat()<rhs.ToFloat());
            case TokenType::GreaterThan:
                return AstValue::MakeBool(lhs.ToFloat()>rhs.ToFloat());
            case TokenType::LessThanOrEqual:
                return AstValue::MakeBool(lhs.ToFloat()<=rhs.ToFloat());
            case TokenType::GreaterThanOrEqual:
                return AstValue::MakeBool(lhs.ToFloat()>=rhs.ToFloat());
            case TokenType::And:
                return AstValue::MakeBool(lhs.ToBool() && rhs.ToBool());
            case TokenType::Or:
                return AstValue::MakeBool(lhs.ToBool() || rhs.ToBool());
            default:
                ctx.error="unsupported binary operator";
                return AstValue::MakeVoid();
        }
    }

    AstValue CallExpr::Eval(ExecContext &ctx) const
    {
        std::vector<AstValue> values;
        values.reserve(args.size());
        for(const auto &arg:args)
        {
            AstValue v=arg->Eval(ctx);
            if(!ctx.error.empty())
                return AstValue::MakeVoid();
            values.push_back(std::move(v));
        }

        if(!ctx.module)
        {
            ctx.error="missing module";
            return AstValue::MakeVoid();
        }

        if(FuncMap *map=ctx.module->GetFuncMap(name))
            return CallNative(map,values,ctx);

        if(Func *func=ctx.module->GetScriptFunc(name))
        {
            if(!ctx.context)
            {
                ctx.error="missing context for script call";
                return AstValue::MakeVoid();
            }
            return ctx.context->ExecuteFunction(func,nullptr);
        }

        ctx.error="unknown function: "+name;
        return AstValue::MakeVoid();
    }

    ExecResult VarDeclStmt::Exec(ExecContext &ctx) const
    {
        if(ctx.locals.find(name)!=ctx.locals.end())
            return ExecResult::Error("duplicate variable: "+name);

        AstValue value=MakeDefaultValue(type);
        if(init)
        {
            value=init->Eval(ctx);
            if(!ctx.error.empty())
                return ExecResult::Error(ctx.error);
        }

        ctx.locals.emplace(name,CastValue(value,type));
        return ExecResult::Normal();
    }

    ExecResult AssignStmt::Exec(ExecContext &ctx) const
    {
        auto it=ctx.locals.find(name);
        if(it==ctx.locals.end())
            return ExecResult::Error("unknown variable: "+name);

        AstValue value=this->value->Eval(ctx);
        if(!ctx.error.empty())
            return ExecResult::Error(ctx.error);

        it->second=CastValue(value,it->second.type);
        return ExecResult::Normal();
    }

    ExecResult ExprStmt::Exec(ExecContext &ctx) const
    {
        expr->Eval(ctx);
        if(!ctx.error.empty())
            return ExecResult::Error(ctx.error);
        return ExecResult::Normal();
    }

    ExecResult ReturnStmt::Exec(ExecContext &ctx) const
    {
        if(!expr)
            return ExecResult::Return(AstValue::MakeVoid());

        AstValue value=expr->Eval(ctx);
        if(!ctx.error.empty())
            return ExecResult::Error(ctx.error);
        return ExecResult::Return(std::move(value));
    }

    ExecResult GotoStmt::Exec(ExecContext &) const
    {
        return ExecResult::Goto(label);
    }

    ExecResult LabelStmt::Exec(ExecContext &) const
    {
        return ExecResult::Normal();
    }

    ExecResult IfStmt::Exec(ExecContext &ctx) const
    {
        AstValue cond_value=cond->Eval(ctx);
        if(!ctx.error.empty())
            return ExecResult::Error(ctx.error);

        if(cond_value.ToBool())
            return then_block->Exec(ctx);

        if(else_block)
            return else_block->Exec(ctx);

        return ExecResult::Normal();
    }

    ExecResult WhileStmt::Exec(ExecContext &ctx) const
    {
        while(true)
        {
            AstValue cond_value=cond->Eval(ctx);
            if(!ctx.error.empty())
                return ExecResult::Error(ctx.error);
            if(!cond_value.ToBool())
                return ExecResult::Normal();

            ExecResult res=body->Exec(ctx);
            if(res.flow!=ExecFlow::Normal)
                return res;
        }
    }

    ExecResult ForStmt::Exec(ExecContext &ctx) const
    {
        (void)ctx;
        return ExecResult::Error("for statement is not implemented");
    }

    ExecResult SwitchStmt::Exec(ExecContext &ctx) const
    {
        (void)ctx;
        return ExecResult::Error("switch statement is not implemented");
    }

    ExecResult EnumDeclStmt::Exec(ExecContext &ctx) const
    {
        (void)ctx;
        return ExecResult::Error("enum declaration is not implemented");
    }

    ExecResult BlockStmt::Exec(ExecContext &ctx) const
    {
        for(const auto &stmt:statements)
        {
            ExecResult res=stmt->Exec(ctx);
            if(res.flow!=ExecFlow::Normal)
                return res;
        }
        return ExecResult::Normal();
    }
}
