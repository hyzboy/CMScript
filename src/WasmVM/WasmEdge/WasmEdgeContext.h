#pragma once

#include <hgl/wasm/WasmContext.h>
#include "WasmEdgeModule.h"

// Forward declarations for WasmEdge C API types
struct WasmEdge_VMContext;
struct WasmEdge_StoreContext;
struct WasmEdge_ModuleInstanceContext;

namespace hgl::wasm
{
    /**
     * WasmEdge执行上下文实现
     */
    class WasmEdgeContext : public IWasmContext
    {
    private:
        std::shared_ptr<IWasmModule> module_;
        WasmEdge_VMContext* vm_context_;
        WasmEdge_ModuleInstanceContext* module_inst_;
        std::string error_message_;

    public:
        WasmEdgeContext();
        virtual ~WasmEdgeContext();

        bool Instantiate(std::shared_ptr<IWasmModule> module) override;
        
        bool CallFunction(
            const char* func_name,
            const Value* args,
            size_t argc,
            Value* results,
            size_t result_count) override;

        bool CallFunction(const char* func_name) override;
        bool CallFunctionI32(const char* func_name, int32_t arg, int32_t& result) override;

        uint8_t* GetMemory(const char* memory_name = "memory") override;
        size_t GetMemorySize(const char* memory_name = "memory") override;

        std::string GetErrorMessage() const override { return error_message_; }
        VMType GetVMType() const override { return VMType::WasmEdge; }

    private:
        void SetError(const std::string& msg);
        enum WasmEdge_ValType ConvertValueType(ValueType type);
    };

}//namespace hgl::wasm
