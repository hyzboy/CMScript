#include <hgl/wasm/WasmVM.h>

#include "WAMRModule.h"
#include "WAMRContext.h"
extern "C"
{
#include <wasm_export.h>
}

namespace hgl::wasm
{
    // Global initialization state
    static bool wamr_initialized = false;
    static bool wasmedge_initialized = false;

    bool InitializeVM()
    {
        if (!wamr_initialized)
        {
            if (!wasm_runtime_init())
                return false;
            wamr_initialized = true;
        }
        return true;
    }

    void CleanupVM()
    {
        if (wamr_initialized)
        {
            wasm_runtime_destroy();
            wamr_initialized = false;
        }
    }

    std::shared_ptr<IWasmModule> CreateWasmModule()
    {
        if (!wamr_initialized)
        {
            if (!InitializeVM())
                return nullptr;
        }
        return std::make_shared<WAMRModule>();
    }

    std::shared_ptr<IWasmContext> CreateWasmContext()
    {
        if (!wamr_initialized)
        {
            if (!InitializeVM())
                return nullptr;
        }
        return std::make_shared<WAMRContext>();
    }
}//namespace hgl::wasm
