#ifndef DevilParseH
#define DevilParseH

#include"as_tokenizer.h"
#include"DevilFunc.h"
#include<hgl/type/String.h>
#include<hgl/platform/compiler/EventFunc.h>
#include<hgl/log/Log.h>

using namespace angle_script;
namespace hgl
{
namespace devil
{
    class Parse
    {
        OBJECT_LOGGER

        Module * module;

        const u16char *     source_start;

        const u16char *     source_cur;
        uint                source_length;

        asCTokenizer        parse;

    private:

        bool                    ParseCode(Func *);                                             //解析一段代码

        template<typename T>
        bool                    ParseNumber(T &,const U16String &);

        ValueInterface *   ParseValue();                                                       //解析一个量(属性/数值/真实函数调用)
        void                    ParseValue(Func *,eTokenType,U16String &);
        void                    ParseEnum();

        #ifdef _DEBUG
        Command *          ParseFuncCall(U16String &,FuncMap *,U16String &);
        #else
        Command *          ParseFuncCall(FuncMap *);
        #endif//
        bool                    ParseIf(Func *);

        CompInterface *    ParseComp();
        eTokenType              ParseCompType();

    public:

        Parse(Module *,const u16char *,int=-1);

        eTokenType GetToken(U16String &);     //取得一个token,自动跳过注释、换行、空格
        eTokenType CheckToken(U16String &);   //检测下一个token,自动跳过注释、换行、空格,但不取出

        bool GetToken(eTokenType,U16String &);    //找某一种Token为止

        bool ParseFunc(Func *);        //解析一个函数
    };
    }//namespace devil
    }//namespace hgl
#endif
