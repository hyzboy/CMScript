#include "DevilAst.h"
#include "DevilCommand.h"
#include "DevilFunc.h"
#include <hgl/devil/DevilModule.h>
#include <hgl/devil/DevilContext.h>
#include <hgl/devil/DevilValue.h>
#include <cmath>

namespace hgl::devil
{
    namespace
    {
        constexpr size_t kMaxLoopIterations=1000;
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
                case TokenType::Double: return AstValue::MakeDouble(0.0);
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
                case TokenType::Double: return AstValue::MakeDouble(value.ToDouble());
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
                case TokenType::Double:
                    out_param.d=value.ToDouble();
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

        bool SwitchEqual(const AstValue &lhs,const AstValue &rhs)
        {
            if(lhs.type==TokenType::String || rhs.type==TokenType::String)
                return lhs.ToString()==rhs.ToString();

            if(lhs.IsNumeric() && rhs.IsNumeric())
                return lhs.ToDouble()==rhs.ToDouble();

            return lhs.ToString()==rhs.ToString();
        }
    }

    AstValue AstValue::MakeVoid(){return AstValue{};}
    AstValue AstValue::MakeBool(bool v){AstValue r; r.type=TokenType::Bool; r.data.b=v; return r;}
    AstValue AstValue::MakeInt(int32_t v){AstValue r; r.type=TokenType::Int; r.data.i=v; return r;}
    AstValue AstValue::MakeUInt(uint32_t v){AstValue r; r.type=TokenType::UInt; r.data.u=v; return r;}
    AstValue AstValue::MakeFloat(float v){AstValue r; r.type=TokenType::Float; r.data.f=v; return r;}
    AstValue AstValue::MakeDouble(double v){AstValue r; r.type=TokenType::Double; r.data.d=v; return r;}
    AstValue AstValue::MakeString(std::string v){AstValue r; r.type=TokenType::String; r.s=std::move(v); return r;}

    bool AstValue::IsNumeric() const
    {
        return type==TokenType::Bool || type==TokenType::Int || type==TokenType::UInt || type==TokenType::Float
            || type==TokenType::Double || type==TokenType::Int8 || type==TokenType::Int16
            || type==TokenType::UInt8 || type==TokenType::UInt16;
    }

    bool AstValue::ToBool() const
    {
        if(type==TokenType::Bool)
            return data.b;
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return data.i!=0;
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return data.u!=0u;
        if(type==TokenType::Float)
            return std::fabs(data.f)>0.000001f;
        if(type==TokenType::Double)
            return std::fabs(data.d)>0.0000001;
        return false;
    }

    int32_t AstValue::ToInt() const
    {
        if(type==TokenType::Bool)
            return data.b?1:0;
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return data.i;
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return static_cast<int32_t>(data.u);
        if(type==TokenType::Float)
            return static_cast<int32_t>(data.f);
        if(type==TokenType::Double)
            return static_cast<int32_t>(data.d);
        return 0;
    }

    uint32_t AstValue::ToUInt() const
    {
        if(type==TokenType::Bool)
            return data.b?1u:0u;
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return static_cast<uint32_t>(data.i);
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return data.u;
        if(type==TokenType::Float)
            return static_cast<uint32_t>(data.f);
        if(type==TokenType::Double)
            return static_cast<uint32_t>(data.d);
        return 0u;
    }

    float AstValue::ToFloat() const
    {
        if(type==TokenType::Bool)
            return data.b?1.0f:0.0f;
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return static_cast<float>(data.i);
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return static_cast<float>(data.u);
        if(type==TokenType::Float)
            return data.f;
        if(type==TokenType::Double)
            return static_cast<float>(data.d);
        return 0.0f;
    }

    double AstValue::ToDouble() const
    {
        if(type==TokenType::Bool)
            return data.b?1.0:0.0;
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return static_cast<double>(data.i);
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return static_cast<double>(data.u);
        if(type==TokenType::Float)
            return static_cast<double>(data.f);
        if(type==TokenType::Double)
            return data.d;
        return 0.0;
    }

    std::string AstValue::ToString() const
    {
        if(type==TokenType::String)
            return s;
        if(type==TokenType::Bool)
            return data.b?"true":"false";
        if(type==TokenType::Int || type==TokenType::Int8 || type==TokenType::Int16)
            return std::to_string(data.i);
        if(type==TokenType::UInt || type==TokenType::UInt8 || type==TokenType::UInt16)
            return std::to_string(data.u);
        if(type==TokenType::Float)
            return std::to_string(data.f);
        if(type==TokenType::Double)
            return std::to_string(data.d);
        return std::string();
    }

    ExecResult ExecResult::Normal(){return ExecResult{ExecFlow::Normal,AstValue::MakeVoid(),{}, {}};}
    ExecResult ExecResult::Return(AstValue v){return ExecResult{ExecFlow::Return,std::move(v),{}, {}};}
    ExecResult ExecResult::Goto(const std::string &label){return ExecResult{ExecFlow::Goto,AstValue::MakeVoid(),label,{}};}
    ExecResult ExecResult::Break(){return ExecResult{ExecFlow::Break,AstValue::MakeVoid(),{}, {}};}
    ExecResult ExecResult::Continue(){return ExecResult{ExecFlow::Continue,AstValue::MakeVoid(),{}, {}};}
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
                if(val.type==TokenType::Double)
                    return AstValue::MakeDouble(-val.ToDouble());
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
                {
                    if(lhs.type==TokenType::Double || rhs.type==TokenType::Double)
                        return AstValue::MakeDouble(lhs.ToDouble()+rhs.ToDouble());
                    return AstValue::MakeFloat(lhs.ToFloat()+rhs.ToFloat());
                }
                ctx.error="'+' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Minus:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                {
                    if(lhs.type==TokenType::Double || rhs.type==TokenType::Double)
                        return AstValue::MakeDouble(lhs.ToDouble()-rhs.ToDouble());
                    return AstValue::MakeFloat(lhs.ToFloat()-rhs.ToFloat());
                }
                ctx.error="'-' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Star:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                {
                    if(lhs.type==TokenType::Double || rhs.type==TokenType::Double)
                        return AstValue::MakeDouble(lhs.ToDouble()*rhs.ToDouble());
                    return AstValue::MakeFloat(lhs.ToFloat()*rhs.ToFloat());
                }
                ctx.error="'*' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Slash:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                {
                    if(lhs.type==TokenType::Double || rhs.type==TokenType::Double)
                        return AstValue::MakeDouble(lhs.ToDouble()/rhs.ToDouble());
                    return AstValue::MakeFloat(lhs.ToFloat()/rhs.ToFloat());
                }
                ctx.error="'/' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Percent:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeInt(rhs.ToInt()!=0?lhs.ToInt()%rhs.ToInt():0);
                ctx.error="'%' expects numeric";
                return AstValue::MakeVoid();
            case TokenType::Equal:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeBool(lhs.ToDouble()==rhs.ToDouble());
                return AstValue::MakeBool(lhs.ToString()==rhs.ToString());
            case TokenType::NotEqual:
                if(lhs.IsNumeric() && rhs.IsNumeric())
                    return AstValue::MakeBool(lhs.ToDouble()!=rhs.ToDouble());
                return AstValue::MakeBool(lhs.ToString()!=rhs.ToString());
            case TokenType::LessThan:
                return AstValue::MakeBool(lhs.ToDouble()<rhs.ToDouble());
            case TokenType::GreaterThan:
                return AstValue::MakeBool(lhs.ToDouble()>rhs.ToDouble());
            case TokenType::LessThanOrEqual:
                return AstValue::MakeBool(lhs.ToDouble()<=rhs.ToDouble());
            case TokenType::GreaterThanOrEqual:
                return AstValue::MakeBool(lhs.ToDouble()>=rhs.ToDouble());
            case TokenType::And:
                return AstValue::MakeBool(lhs.ToBool() && rhs.ToBool());
            case TokenType::Or:
                return AstValue::MakeBool(lhs.ToBool() || rhs.ToBool());
            case TokenType::Amp:
                return AstValue::MakeInt(lhs.ToInt() & rhs.ToInt());
            case TokenType::BitOr:
                return AstValue::MakeInt(lhs.ToInt() | rhs.ToInt());
            case TokenType::BitXor:
                return AstValue::MakeInt(lhs.ToInt() ^ rhs.ToInt());
            case TokenType::BitShiftLeft:
                return AstValue::MakeInt(lhs.ToInt() << rhs.ToInt());
            case TokenType::BitShiftRight:
            case TokenType::BitShiftRightArith:
                return AstValue::MakeInt(lhs.ToInt() >> rhs.ToInt());
            default:
                ctx.error="unsupported binary operator";
                return AstValue::MakeVoid();
        }
    }

    AstValue AssignExpr::Eval(ExecContext &ctx) const
    {
        auto it=ctx.locals.find(name);
        if(it==ctx.locals.end())
        {
            ctx.error="unknown variable: "+name;
            return AstValue::MakeVoid();
        }

        AstValue rhs=value->Eval(ctx);
        if(!ctx.error.empty())
            return AstValue::MakeVoid();

        it->second=CastValue(rhs,it->second.type);
        return it->second;
    }

    AstValue CastExpr::Eval(ExecContext &ctx) const
    {
        AstValue rhs=value->Eval(ctx);
        if(!ctx.error.empty())
            return AstValue::MakeVoid();

        return CastValue(rhs,target_type);
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
            const auto &params=func->GetParams();
            if(values.size()!=params.size())
            {
                ctx.error="script call param count mismatch";
                return AstValue::MakeVoid();
            }
            return ctx.context->ExecuteFunction(func,nullptr,values);
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
        size_t iterations=0;
        while(true)
        {
            if(++iterations>kMaxLoopIterations)
                return ExecResult::Error("loop iteration limit exceeded");

            AstValue cond_value=cond->Eval(ctx);
            if(!ctx.error.empty())
                return ExecResult::Error(ctx.error);
            if(!cond_value.ToBool())
                return ExecResult::Normal();

            ExecResult res=body->Exec(ctx);
            if(res.flow==ExecFlow::Normal)
                continue;
            if(res.flow==ExecFlow::Continue)
                continue;
            if(res.flow==ExecFlow::Break)
                return ExecResult::Normal();
            return res;
        }
    }

    ExecResult DoWhileStmt::Exec(ExecContext &ctx) const
    {
        size_t iterations=0;
        while(true)
        {
            if(++iterations>kMaxLoopIterations)
                return ExecResult::Error("loop iteration limit exceeded");

            ExecResult res=body->Exec(ctx);
            if(res.flow==ExecFlow::Return || res.flow==ExecFlow::Goto || res.flow==ExecFlow::Error)
                return res;
            if(res.flow==ExecFlow::Break)
                return ExecResult::Normal();

            AstValue cond_value=cond->Eval(ctx);
            if(!ctx.error.empty())
                return ExecResult::Error(ctx.error);
            if(!cond_value.ToBool())
                return ExecResult::Normal();
        }
    }

    ExecResult ForStmt::Exec(ExecContext &ctx) const
    {
        if(init)
        {
            ExecResult init_res=init->Exec(ctx);
            if(init_res.flow!=ExecFlow::Normal)
                return init_res;
        }

        size_t iterations=0;
        while(true)
        {
            if(++iterations>kMaxLoopIterations)
                return ExecResult::Error("loop iteration limit exceeded");

            if(cond)
            {
                AstValue cond_value=cond->Eval(ctx);
                if(!ctx.error.empty())
                    return ExecResult::Error(ctx.error);
                if(!cond_value.ToBool())
                    return ExecResult::Normal();
            }

            ExecResult res=body->Exec(ctx);
            if(res.flow==ExecFlow::Return || res.flow==ExecFlow::Goto || res.flow==ExecFlow::Error)
                return res;
            if(res.flow==ExecFlow::Break)
                return ExecResult::Normal();

            if(post)
            {
                post->Eval(ctx);
                if(!ctx.error.empty())
                    return ExecResult::Error(ctx.error);
            }

            if(res.flow==ExecFlow::Continue)
                continue;
        }
    }

    ExecResult SwitchStmt::Exec(ExecContext &ctx) const
    {
        AstValue switch_value=expr->Eval(ctx);
        if(!ctx.error.empty())
            return ExecResult::Error(ctx.error);

        const auto &case_list=cases;
        size_t match_index=case_list.size();
        for(size_t i=0;i<case_list.size();++i)
        {
            AstValue case_value=case_list[i].expr->Eval(ctx);
            if(!ctx.error.empty())
                return ExecResult::Error(ctx.error);
            if(SwitchEqual(switch_value,case_value))
            {
                match_index=i;
                break;
            }
        }

        if(match_index<case_list.size())
        {
            for(size_t i=match_index;i<case_list.size();++i)
            {
                ExecResult res=case_list[i].block->Exec(ctx);
                if(res.flow==ExecFlow::Normal)
                    continue;
                if(res.flow==ExecFlow::Break)
                    return ExecResult::Normal();
                return res;
            }
        }

        if(default_block)
        {
            ExecResult res=default_block->Exec(ctx);
            if(res.flow==ExecFlow::Break)
                return ExecResult::Normal();
            return res;
        }

        return ExecResult::Normal();
    }

    ExecResult BreakStmt::Exec(ExecContext &ctx) const
    {
        (void)ctx;
        return ExecResult::Break();
    }

    ExecResult ContinueStmt::Exec(ExecContext &ctx) const
    {
        (void)ctx;
        return ExecResult::Continue();
    }

    ExecResult EnumDeclStmt::Exec(ExecContext &ctx) const
    {
        (void)ctx;
        return ExecResult::Error("enum declaration is not implemented");
    }

    ExecResult BlockStmt::Exec(ExecContext &ctx) const
    {
        for(size_t i=0;i<statements.size();++i)
        {
            if(i<statement_locations.size())
                ctx.current_loc=statement_locations[i];
            else
                ctx.current_loc=SourceLocation{};

            ExecResult res=statements[i]->Exec(ctx);
            if(res.flow!=ExecFlow::Normal)
                return res;
        }
        return ExecResult::Normal();
    }
}
