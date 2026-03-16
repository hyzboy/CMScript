#include <hgl/wasm/WasmVM.h>

#ifdef CMSCRIPT_WAMR_ENABLED
#include "WAMR/WAMRModule.h"
#include "WAMR/WAMRContext.h"
extern "C"
{
#include <wasm_export.h>
}
#endif

#ifdef CMSCRIPT_WASMEDGE_ENABLED
#include "WasmEdge/WasmEdgeModule.h"
#include "WasmEdge/WasmEdgeContext.h"
#endif

namespace hgl::wasm
{
    // Global initialization state
    static bool wamr_initialized = false;
    static bool wasmedge_initialized = false;

    bool InitializeVM(VMType type)
    {
        switch (type)
        {
#ifdef CMSCRIPT_WAMR_ENABLED
            case VMType::WAMR:
            {
                if (!wamr_initialized)
                {
                    if (!wasm_runtime_init())
                        return false;
                    wamr_initialized = true;
                }
                return true;
            }
#endif
#ifdef CMSCRIPT_WASMEDGE_ENABLED
            case VMType::WasmEdge:
            {
                // WasmEdge doesn't require global initialization
                wasmedge_initialized = true;
                return true;
            }
#endif
            default:
                return false;
        }
    }

    void CleanupVM(VMType type)
    {
        switch (type)
        {
#ifdef CMSCRIPT_WAMR_ENABLED
            case VMType::WAMR:
            {
                if (wamr_initialized)
                {
                    wasm_runtime_destroy();
                    wamr_initialized = false;
                }
                break;
            }
#endif
#ifdef CMSCRIPT_WASMEDGE_ENABLED
            case VMType::WasmEdge:
            {
                // WasmEdge doesn't require global cleanup
                wasmedge_initialized = false;
                break;
            }
#endif
        }
    }

    std::shared_ptr<IWasmModule> CreateWasmModule(VMType type)
    {
        switch (type)
        {
#ifdef CMSCRIPT_WAMR_ENABLED
            case VMType::WAMR:
            {
                if (!wamr_initialized)
                {
                    if (!InitializeVM(VMType::WAMR))
                        return nullptr;
                }
                return std::make_shared<WAMRModule>();
            }
#endif
#ifdef CMSCRIPT_WASMEDGE_ENABLED
            case VMType::WasmEdge:
            {
                if (!wasmedge_initialized)
                {
                    if (!InitializeVM(VMType::WasmEdge))
                        return nullptr;
                }
                return std::make_shared<WasmEdgeModule>();
            }
#endif
            default:
                return nullptr;
        }
    }

    std::shared_ptr<IWasmContext> CreateWasmContext(VMType type)
    {
        switch (type)
        {
#ifdef CMSCRIPT_WAMR_ENABLED
            case VMType::WAMR:
            {
                if (!wamr_initialized)
                {
                    if (!InitializeVM(VMType::WAMR))
                        return nullptr;
                }
                return std::make_shared<WAMRContext>();
            }
#endif
#ifdef CMSCRIPT_WASMEDGE_ENABLED
            case VMType::WasmEdge:
            {
                if (!wasmedge_initialized)
                {
                    if (!InitializeVM(VMType::WasmEdge))
                        return nullptr;
                }
                return std::make_shared<WasmEdgeContext>();
            }
#endif
            default:
                return nullptr;
        }
    }

}//namespace hgl::wasm
