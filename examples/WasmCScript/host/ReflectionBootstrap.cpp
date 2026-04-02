#include "ReflectionBootstrap.h"
#include "GameHost.reflect.h"

bool register_all_script_bindings() {
    return h_reflect_register_gamehost();
}

#if defined(H_REFLECT_AUTO_REGISTER)
namespace {
struct AutoScriptBindingRegister {
    AutoScriptBindingRegister() {
        (void)register_all_script_bindings();
    }
};

AutoScriptBindingRegister g_auto_script_binding_register;
}  // namespace
#endif
