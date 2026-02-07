#pragma once

#include"as_tokenizer.h"
#include"DevilFunc.h"
#include <string>
#include<hgl/platform/compiler/EventFunc.h>
#include<hgl/log/Log.h>

namespace hgl::devil
{
    using namespace angle_script;

    class Parse
    {
        OBJECT_LOGGER

        Module * module;

        const char *        source_start;

        const char *        source_cur;
        uint                source_length;

        asCTokenizer        parse;

    private:

        bool                    ParseCode(Func *);                                             //解析一段代码

        template<typename T>
        bool                    ParseNumber(T &,const std::string &);

        ValueInterface *   ParseValue();                                                       //解析一个量(属性/数值/真实函数调用)
        void                    ParseValue(Func *,eTokenType,std::string &);
        void                    ParseEnum();

        #ifdef _DEBUG
        Command *          ParseFuncCall(std::string &,FuncMap *,std::string &);
        #else
        Command *          ParseFuncCall(FuncMap *);
        #endif//
        bool                    ParseIf(Func *);

        CompInterface *    ParseComp();
        eTokenType              ParseCompType();

    public:

        Parse(Module *,const char *,int=-1);

        eTokenType GetToken(std::string &);     //取得一个token,自动跳过注释、换行、空格
        eTokenType CheckToken(std::string &);   //检测下一个token,自动跳过注释、换行、空格,但不取出

        bool GetToken(eTokenType,std::string &);    //找某一种Token为止

        bool ParseFunc(Func *);        //解析一个函数
    };
}//namespace hgl::devil