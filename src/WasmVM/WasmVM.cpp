#include <hgl/wasm/WasmVM.h>
#include "WAMR/WAMRModule.h"
#include "WAMR/WAMRContext.h"
#include "WasmEdge/WasmEdgeModule.h"
#include "WasmEdge/WasmEdgeContext.h"
#include <wasm_export.h>

namespace hgl::wasm
{
    // Global initialization state
    static bool wamr_initialized = false;
    static bool wasmedge_initialized = false;

    bool InitializeVM(VMType type)
    {
        switch (type)
        {
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
            case VMType::WasmEdge:
            {
                // WasmEdge doesn't require global initialization
                wasmedge_initialized = true;
                return true;
            }
            default:
                return false;
        }
    }

    void CleanupVM(VMType type)
    {
        switch (type)
        {
            case VMType::WAMR:
            {
                if (wamr_initialized)
                {
                    wasm_runtime_destroy();
                    wamr_initialized = false;
                }
                break;
            }
            case VMType::WasmEdge:
            {
                // WasmEdge doesn't require global cleanup
                wasmedge_initialized = false;
                break;
            }
        }
    }

    std::shared_ptr<IWasmModule> CreateWasmModule(VMType type)
    {
        switch (type)
        {
            case VMType::WAMR:
            {
                if (!wamr_initialized)
                {
                    if (!InitializeVM(VMType::WAMR))
                        return nullptr;
                }
                return std::make_shared<WAMRModule>();
            }
            case VMType::WasmEdge:
            {
                if (!wasmedge_initialized)
                {
                    if (!InitializeVM(VMType::WasmEdge))
                        return nullptr;
                }
                return std::make_shared<WasmEdgeModule>();
            }
            default:
                return nullptr;
        }
    }

    std::shared_ptr<IWasmContext> CreateWasmContext(VMType type)
    {
        switch (type)
        {
            case VMType::WAMR:
            {
                if (!wamr_initialized)
                {
                    if (!InitializeVM(VMType::WAMR))
                        return nullptr;
                }
                return std::make_shared<WAMRContext>();
            }
            case VMType::WasmEdge:
            {
                if (!wasmedge_initialized)
                {
                    if (!InitializeVM(VMType::WasmEdge))
                        return nullptr;
                }
                return std::make_shared<WasmEdgeContext>();
            }
            default:
                return nullptr;
        }
    }

}//namespace hgl::wasm
