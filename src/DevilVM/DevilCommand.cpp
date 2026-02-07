#include"DevilCommand.h"
#include"DevilContext.h"
#include"DevilFunc.h"

namespace hgl
{
namespace devil
{
#if HGL_CPU == HGL_CPU_X86_32
    void *CallCDeclFunction(void *,const void *,int);                                           ///<呼叫C函数
    void *CallThiscallFunction(void *,const void *,const void *,int);                           ///<呼叫C++函数

    bool FuncMap::Call(const SystemFuncParam *call_param,const int param_size,void *return_result)
    {
        if(base)        //有this指针
            *(void **)return_result=CallThiscallFunction(func,base,call_param,param_size);
        else
            *(void **)return_result=CallCDeclFunction(func,call_param,param_size);

        return(true);
    }
#elif HGL_CPU == HGL_CPU_X86_64
    extern "C" void *CallX64(void *func,int argc,const void *argv, const void *argv_float);

    bool FuncMap::Call(const SystemFuncParam *call_param,const int param_size,void *return_result)
    {
        //X86-64位情况下，this放入参数的第一个，在解析代码时已经确定，所以无需再次处理
        *(void **)return_result=CallX64(func,param_size,call_param,call_param);

        return(true);
    }
#endif//
}//namespace devil
}//namespace hgl

//namespace hgl
//{
//  SystemFuncCallDynamic::SystemFuncCallDynamic(FuncMap *dfm)
//  {
//      func=dfm;
//
//      param_size=dfm->param.Count*sizeof(uint);
//      param=new uint[dfm->param.Count];
//  }
//
//  SystemFuncCallDynamic::~SystemFuncCallDynamic()
//  {
//      delete[] param;
//  }
//
//  bool SystemFuncCallDynamic::Run(Context *context)
//  {
//
//  }
//}//namespace hgl

namespace hgl
{
namespace devil
{
    ScriptFuncCall::ScriptFuncCall(Module *dm,Func *df)
    {
        module=dm;
        func=df;
    }

    bool ScriptFuncCall::Run(Context *context)
    {
        context->ScriptFuncCall(func);

        return(true);
    }
}//namespace devil
}//namespace hgl

namespace hgl
{
namespace devil
{
    Goto::Goto(Module *dm,Func *df,const U16String &flag)
    {
        module=dm;

        func=df;

        name=flag;

        index=-1;
    }

    void Goto::UpdateGotoFlag()
    {
        func->goto_flag.Get(name,index);        // 由于跳转标识有可能在这个GOTO之后定义，所以必须等这个函数解晰完了，再调用SetLine

        if(index==-1)
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("在函数<")+func->func_name+U16_TEXT(">没有找到跳转标识:")+name)
                         .c_str());
    }

    bool Goto::Run(Context *context)
    {
        #ifdef _DEBUG
            LogInfo(U16_TEXT("%s"),
                    (U16_TEXT("在函数<")+func->func_name+U16_TEXT(">中跳转:")+name)
                        .c_str());
        #endif//_DEBUG

        return context->Goto(func,index);
    }
}//namespace devil
}//namespace hgl

namespace hgl
{
namespace devil
{
    CompGoto::CompGoto(Module *dm,CompInterface *dci,Func *f)
    {
        module=dm;
        comp=dci;
        func=f;

        index=-1;
    }

    CompGoto::~CompGoto()
    {
        delete comp;
    }

    void CompGoto::UpdateGotoFlag()
    {
        func->goto_flag.Get(else_flag,index);

        if(index==-1)
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("在函数<")+func->func_name+U16_TEXT(">没有找到跳转标识:")+else_flag).c_str());
    }

    bool CompGoto::Run(Context *context)
    {
        if(comp->Comp())return(true);

        if(index==-1)           //不含else的if脚本，else_flag自动为end_flag
            return(false);

        return context->Goto(func,index);
    }
}//namespace devil
}//namespace hgl

namespace hgl
{
namespace devil
{
    Return::Return(Module *dm)
    {
        module=dm;
    }

    bool Return::Run(Context *context)
    {
        return context->Return();
    }
}//namespace devil
}//namespace hgl

