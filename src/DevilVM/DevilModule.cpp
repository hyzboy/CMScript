#include <hgl/devil/DevilModule.h>
#include "DevilCommand.h"
#include "DevilParse.h"
#include "DevilFunc.h"
#include <cstring>

namespace hgl
{
namespace devil
{
    namespace
    {
        TokenType ToToken(detail::BindType type)
        {
            switch(type)
            {
                case detail::BindType::Void:   return TokenType::Void;
                case detail::BindType::Bool:   return TokenType::Bool;
                case detail::BindType::Int:    return TokenType::Int;
                case detail::BindType::Int8:   return TokenType::Int8;
                case detail::BindType::Int16:  return TokenType::Int16;
                case detail::BindType::UInt:   return TokenType::UInt;
                case detail::BindType::UInt8:  return TokenType::UInt8;
                case detail::BindType::UInt16: return TokenType::UInt16;
                case detail::BindType::Float:  return TokenType::Float;
                case detail::BindType::String: return TokenType::String;
            }

            return TokenType::Void;
        }
    }//namespace

    /**
    * 映射一个属性
    * @param intro 属性在脚本语言中的描述,如"int value","string name"等
    * @param address 属性的地址
    * @return 是否创建映射成功
    */
    bool Module::MapProperty(const char *intro,void *address)
    {
        Parse parse(this,intro);
        TokenType type;
        std::string name;

        type=parse.GetToken(name);

        parse.GetToken(name);

        if(prop_map.find(name)!=prop_map.end())     //在已有记录找到同名的映射
        {
            LogError("%s",
                     ("重复属性名映射: "+std::string(intro)).c_str());
            return(false);
        }
        else
        {
                LogInfo("%s",
                    ("映射属性: "+std::string(intro)).c_str());
        }

        {
            PropertyMap *dpm=new PropertyMap;

            dpm->type=type;
            dpm->address=address;

            prop_map.emplace(name,dpm);

            return(true);
        }
    }

    bool Module::_MapFuncTyped(const char *name,void *this_pointer,void *func_pointer,detail::BindType result,std::initializer_list<detail::BindType> params)
    {
        if(!name||!(*name))
            return(false);

        if(func_map.find(name)!=func_map.end())
        {
            LogError("%s",
                     ("repeat func name:"+std::string(name)).c_str());
            return(false);
        }

        FuncMap *dfm=new FuncMap;

        dfm->base=this_pointer;
        dfm->func=func_pointer;
        dfm->result=ToToken(result);

        dfm->param.reserve(params.size());
        for(const detail::BindType type:params)
            dfm->param.push_back(ToToken(type));

        func_map.emplace(name,dfm);

        #ifdef _DEBUG
        {
            std::string func_intro=std::string(GetTokenName(dfm->result)) + " " + name + "(";

            for(size_t i=0;i<dfm->param.size();++i)
            {
                if(i>0)
                    func_intro.push_back(',');

                func_intro+=GetTokenName(dfm->param[i]);
            }

            func_intro.push_back(')');

            LogInfo("%s",
                ("映射函数成功，参数"
                 +std::to_string(static_cast<int>(dfm->param.size()))
                 +"个:"+func_intro).c_str());
        }
        #endif//_DEBUG

        return(true);
    }

    Func *Module::GetScriptFunc(const std::string &name)
    {
        const auto it=script_func.find(name);
        if(it!=script_func.end())
            return it->second;

           LogError("%s",
               ("没有找到指定脚本函数: "+name).c_str());
        return(nullptr);
    }

        FuncMap *Module::GetFuncMap(const std::string &name)
    {
        const auto it=func_map.find(name);
        if(it!=func_map.end())
            return it->second;
        else
            return(nullptr);
    }

    bool Module::AddEnum(const char *enum_name,EnumDef *script_enum)
    {
        if(enum_map.find(enum_name)!=enum_map.end())
        {
            LogError("%s",
                     ("枚举名称重复: "+std::string(enum_name)).c_str());
            return(false);
        }

        enum_map.emplace(enum_name,script_enum);
        return(true);
    }

    PropertyMap *Module::GetPropertyMap(const std::string &name)
    {
        const auto it=prop_map.find(name);
        if(it!=prop_map.end())
            return it->second;
        else
            return(nullptr);
    }

    /**
    * 添加脚本并编译
    * @param source 脚本
    * @param source_length 脚本长度，-1表示自动检测
    * @return 是否添加并编译成功
    */
    bool Module::AddScript(const char *source,int source_length)
    {
        if(!source)return(false);

        if(source_length==-1)
            source_length=strlen(source);

        if(source_length<1)
            return(false);

        Parse parse(this,source,source_length);
        std::string name;

        while(true)
        {
            TokenType type=parse.GetToken(name);               //不停的通过func关键字查找函数

            if(type==TokenType::Func)
            {
                parse.GetToken(name);                           //取得函数名

                if(script_func.find(name)==script_func.end())   //查找是否有同样的函数名存在
                {
                    Func *func=new Func(this,name);

                    LogInfo("%s",("func "+name+"()\n{").c_str());

                    if(parse.ParseFunc(func))                   //解析函数
                    {
                        script_func.emplace(name,func);

                        LogInfo("%s","}\n");
                    }
                    else
                    {
                        delete func;

                        LogError("%s",("解晰函数失败: "+name).c_str());
                        return(false);
                    }
                }
                else
                {
                    LogError("%s",("脚本函数名称重复: "+name).c_str());
                    return(false);
                }

                continue;
            }//if type == TokenType::Func
            else
            if(type==TokenType::Enum)
            {
//                parse.ParseEnum();
            }//if type == TokenType::Enum
            else
            if(type==TokenType::Const)
            {
            }
            else
                break;
        }//while

        return(true);
    }

    void Module::Clear()
    {
        script_func.clear();
        string_list.clear();
    }

#ifdef _DEBUG
    void Module::LogPropertyList()
    {
        int n=static_cast<int>(prop_map.size());

        LogInfo("%s",("\n映射属性列表数量:"+std::to_string(n)).c_str());

        int i=0;
        for(const auto &kv:prop_map)
        {
            const std::string &name=kv.first;
            const PropertyMap *dpm=kv.second;

            LogInfo("%s",
                ("\t"+std::to_string(i)
                 +"\t"+std::string(GetTokenName(dpm->type))
                 +"\t"+name).c_str());
            ++i;
        }
    }

    void Module::LogMapFuncList()
    {
        int n=static_cast<int>(func_map.size());

        LogInfo("%s",("\n映射函数列表数量:"+std::to_string(n)).c_str());

        int i=0;
        for(const auto &kv:func_map)
        {
            std::string str;
            const std::string &name=kv.first;
            const FuncMap *dfm=kv.second;

            //str.Sprintf(u"\t%d\t%8s %s(",i,GetTokenName(dfm->result),name.wc_str());
            str.push_back('\t');
            str+=std::to_string(i);
            str.push_back('\t');
            str+=GetTokenName(dfm->result);
            str.push_back(' ');
            str+=name;
            str.push_back('(');

            for(int j=0;j<static_cast<int>(dfm->param.size());j++)
            {
                str+=GetTokenName(dfm->param[j]);

                if(j<static_cast<int>(dfm->param.size())-1)
                        str.push_back(',');
            }

                    str.push_back(')');

                    LogInfo("%s",str.c_str());
            ++i;
        }
    }

    void Module::LogScriptFuncList()
    {
        int n=static_cast<int>(script_func.size());

        LogInfo("%s",("\n脚本函数列表数量:"+std::to_string(n)).c_str());

        int i=0;
        for(const auto &kv:script_func)
        {
            const std::string &name=kv.first;

            LogInfo("%s",
                ("\t"+std::to_string(i)+"\t"+name).c_str());
            ++i;
        }
    }
#endif//_DEBUG
}//namespace devil
}//namespace hgl


