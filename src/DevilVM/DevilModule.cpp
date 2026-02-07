#include"DevilModule.h"
#include"DevilParse.h"
#include"DevilFunc.h"

namespace hgl
{
namespace devil
{
    /**
    * 映射一个属性
    * @param intro 属性在脚本语言中的描述,如"int value","string name"等
    * @param address 属性的地址
    * @return 是否创建映射成功
    */
    bool Module::MapProperty(const u16char *intro,void *address)
    {
        Parse parse(this,intro);
        eTokenType type;
        U16String name;

        type=parse.GetToken(name);

        parse.GetToken(name);

        if(prop_map.ContainsKey(name))              //在已有记录找到同名的映射
        {
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("重复属性名映射: ")+U16String(intro)).c_str());
            return(false);
        }
        else
        {
                LogInfo(U16_TEXT("%s"),
                    (U16_TEXT("映射属性: ")+U16String(intro)).c_str());
        }

        {
            PropertyMap *dpm=new PropertyMap;

            dpm->type=type;
            dpm->address=address;

            prop_map.Add(name,dpm);

            return(true);
        }
    }

    bool Module::_MapFunc(const u16char *intro,void *this_pointer,void *func_pointer)
    {
        Parse parse(this,intro);
        eTokenType type;
        FuncMap *dfm;

        U16String name,func_name;

        type=parse.GetToken(name);                      //取得函数返回类型

        parse.GetToken(func_name);                      //取得函数名称

        if(func_map.ContainsKey(func_name))             //在已有记录找到同名的映射
        {
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("repeat func name:")+U16String(intro)).c_str());
            return(false);
        }
//      else DebugLog(u"映射函数<%s>",intro);

        #ifdef _DEBUG
            U16String func_intro;

            func_intro=U16String(GetTokenName(type))+U16_TEXT(" ")+func_name+U16_TEXT("(");
                //.Sprintf(u"%8s %s(",GetTokenName(type),func_name.wc_str());
        #endif//

        dfm=new FuncMap;

        dfm->base   =this_pointer;
        dfm->func   =func_pointer;
        dfm->result =type;

        parse.GetToken(ttOpenParanthesis,name);     //找到左括号为止

        while(true)
        {
            type=parse.GetToken(name);

            switch(type)
            {
                case ttCloseParanthesis:func_map.Add(func_name,dfm);        //找到右括号
                                        #ifdef _DEBUG
                                                func_intro+=U16String::charOf(U16_TEXT(')'));

                                                LogInfo(U16_TEXT("%s"),
                                                    (U16_TEXT("映射函数成功，参数")
                                                     +U16String::numberOf(static_cast<int>(dfm->param.size()))
                                                     +U16_TEXT("个:")+func_intro).c_str());
                                        #endif//_DEBUG
                                        return(true);
                case ttBool:

                case ttInt:
                case ttInt8:
                case ttInt16:
//              case ttInt64:
                case ttUInt:
                case ttUInt8:
                case ttUInt16:
//              case ttUInt64:

                case ttFloat:
//              case ttDouble:

                case ttString:
                                        #ifdef _DEBUG
                                            if(!dfm->param.empty())
                                                func_intro+=U16String::charOf(U16_TEXT(','));

                                            func_intro.Strcat(GetTokenName(type));
                                        #endif//_DEBUG

                                        dfm->param.push_back(type);         //增加一个参数类型项
                                        break;

                case ttEnd:             delete dfm;
                                        return(false);
            }
        }
    }

    /**
    * 映射一个C函数
    * @param intro 函数描述，如“int getvalue(int,string)”，注意不可以写成“int getvalue(int index,string value)”
    * @param func_pointer 函数指针
    * @return 是否映射成功
    */
    bool Module::MapFunc(const u16char *intro,void *func_pointer)
    {
        return _MapFunc(intro,nullptr,func_pointer);
    }

    ///**
    //* 映射一个C函数，并传入一个数据
    //* @param data 传入的数据
    //* @param intro 函数描述，如“int getvalue(int,string)”，注意不可以写成“int getvalue(int index,string value)”
    //* @param func_pointer 函数指针
    //* @return 是否映射成功
    //*/
    //bool Module::MapFunc(void *data,const u16char *intro,void *func_pointer)
    //{
    //  return _MapFunc(FuncMap::fcmFirstObject,   intro,data,func_pointer);
    //}

    /**
    * 映射一个C++成员函数
    * @param intro 函数描述，如“int getvalue(int,string)”，注意不可以写成“int getvalue(int index,string value)”
    * @param this_pointer 对象指针
    * @param func_pointer 函数指针
    * @return 是否映射成功
    */
    bool Module::MapFunc(const u16char *intro,void *this_pointer,void *func_pointer)
    {
        return _MapFunc(intro,this_pointer,func_pointer);
    }

    Func *Module::GetScriptFunc(const U16String &name)
    {
        Func *func;

        if(script_func.Get(name,func))
            return func;

        LogError(U16_TEXT("%s"),
             (U16_TEXT("没有找到指定脚本函数: ")+name).c_str());
        return(nullptr);
    }

    FuncMap *Module::GetFuncMap(const U16String &name)
    {
        FuncMap *func;

        if(func_map.Get(name,func))
            return func;
        else
            return(nullptr);
    }

    bool Module::AddEnum(const u16char *enum_name,Enum *script_enum)
    {
        if(enum_map.ContainsKey(enum_name))
        {
            LogError(U16_TEXT("%s"),
                     (U16_TEXT("枚举名称重复: ")+U16String(enum_name)).c_str());
            return(false);
        }

        enum_map.Add(enum_name,script_enum);
        return(true);
    }

    PropertyMap *Module::GetPropertyMap(const U16String &name)
    {
        PropertyMap *dpm;

        if(prop_map.Get(name,dpm))
            return dpm;
        else
            return(nullptr);
    }

    /**
    * 添加脚本并编译
    * @param source 脚本
    * @param source_length 脚本长度，-1表示自动检测
    * @return 是否添加并编译成功
    */
    bool Module::AddScript(const u16char *source,int source_length)
    {
        if(!source)return(false);

        if(source_length==-1)
            source_length=strlen(source);

        if(source_length<1)
            return(false);

        Parse parse(this,source,source_length);
        U16String name;

        while(true)
        {
            eTokenType type=parse.GetToken(name);               //不停的通过func关键字查找函数

            if(type==ttFunc)
            {
                parse.GetToken(name);                           //取得函数名

                if(!script_func.ContainsKey(name))              //查找是否有同样的函数名存在
                {
                    Func *func=new Func(this,name);

                    LogInfo(U16_TEXT("%s"),(U16_TEXT("func ")+name+U16_TEXT("()\n{")).c_str());

                    if(parse.ParseFunc(func))                   //解析函数
                    {
                        script_func.Add(name,func);

                        LogInfo(U16_TEXT("%s"),U16_TEXT("}\n"));
                    }
                    else
                    {
                        delete func;

                        LogError(U16_TEXT("%s"),(U16_TEXT("解晰函数失败: ")+name).c_str());
                        return(false);
                    }
                }
                else
                {
                    LogError(U16_TEXT("%s"),(U16_TEXT("脚本函数名称重复: ")+name).c_str());
                    return(false);
                }

                continue;
            }//if type == ttFunc
            else
            if(type==ttEnum)
            {
//                parse.ParseEnum();
            }//if type == ttEnum
            else
            if(type==ttConst)
            {
            }
            else
                break;
        }//while

        return(true);
    }

    void Module::Clear()
    {
        script_func.Clear();
        string_list.Clear();
    }

#ifdef _DEBUG
    void Module::LogPropertyList()
    {
        int n=prop_map.GetCount();

        LogInfo(U16_TEXT("%s"),(U16_TEXT("\n映射属性列表数量:")+U16String::numberOf(n)).c_str());

        int i=0;
        for(const auto &kv:prop_map)
        {
            const U16String &name=kv.first;
            const PropertyMap *dpm=kv.second;

            LogInfo(U16_TEXT("%s"),
                (U16_TEXT("\t")+U16String::numberOf(i)
                 +U16_TEXT("\t")+U16String(GetTokenName(dpm->type))
                 +U16_TEXT("\t")+name).c_str());
            ++i;
        }
    }

    void Module::LogMapFuncList()
    {
        int n=func_map.GetCount();

        LogInfo(U16_TEXT("%s"),(U16_TEXT("\n映射函数列表数量:")+U16String::numberOf(n)).c_str());

        int i=0;
        for(const auto &kv:func_map)
        {
            U16String str;
            const U16String &name=kv.first;
            const FuncMap *dfm=kv.second;

            //str.Sprintf(u"\t%d\t%8s %s(",i,GetTokenName(dfm->result),name.wc_str());
            str=U16String::charOf(U16_TEXT('\t'));
            str+=U16String::numberOf(i);
            str+=U16String::charOf(U16_TEXT('\t'));
            str+=U16String(GetTokenName(dfm->result));
            str+=U16String::charOf(U16_TEXT(' '));
            str+=name;
            str+=U16String::charOf(U16_TEXT('('));

            for(int j=0;j<static_cast<int>(dfm->param.size());j++)
            {
                str.Strcat(GetTokenName(dfm->param[j]));

                if(j<static_cast<int>(dfm->param.size())-1)
                    str+=U16String::charOf(U16_TEXT(','));
            }

            str+=U16String::charOf(U16_TEXT(')'));

            LogInfo(U16_TEXT("%s"),str.c_str());
            ++i;
        }
    }

    void Module::LogScriptFuncList()
    {
        int n=script_func.GetCount();

        LogInfo(U16_TEXT("%s"),(U16_TEXT("\n脚本函数列表数量:")+U16String::numberOf(n)).c_str());

        int i=0;
        for(const auto &kv:script_func)
        {
            const U16String &name=kv.first;

            LogInfo(U16_TEXT("%s"),
                (U16_TEXT("\t")+U16String::numberOf(i)+U16_TEXT("\t")+name).c_str());
            ++i;
        }
    }
#endif//_DEBUG
}//namespace devil
}//namespace hgl
