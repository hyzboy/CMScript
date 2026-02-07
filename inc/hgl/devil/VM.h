#ifndef HGL_VM_INCLUDE
#define HGL_VM_INCLUDE

#include<string.h>

/**
* 虚拟机共用要素
*/
namespace hgl
{

    #define vm_method(c,m) vm_method_ptr<sizeof(void (c::*)())>::Convert((void (c::*)())(&c::m))

    template <int>
    struct vm_method_ptr
    {
        template<class M>
        static void *Convert(M obj)
        {
            union u
            {
                M m;
                void *v;
            };

            return ((u *)(&obj))->v;
        }
    };

    #define vm_offset(s,m) ((size_t)(&reinterpret_cast<s*>(100000)->m)-100000)

    //全局用
//  #define BindMemberFunc(name,this_pointer,class,func)    BindFunc(name,this_pointer,vm_method(class,func))
//  #define BindCppFunc(name,this_pointer,class,func)       BindMemberFunc(name,this_pointer,class,func)

    //对象用
    #define BindObject(name,class,create,clear)             BindObjectCreate(name,sizeof(class),create,clear)
    #define BindObjectProperty(name,class,value)            BindProperty(name,vm_offset(class,class::value))
    #define BindObjectArray(name_type,name,class,get,set)   BindArray(name_type,name,vm_method(class,get),vm_method(class,set))
    #define BindObjectFunc(name,class,func)                 BindFunc(name,vm_method(class,func))
}//namespace hgl
#endif//HGL_VM_INCLUDE
