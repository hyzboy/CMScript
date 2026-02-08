#pragma once

#include <hgl/wasm/WasmContext.h>
#include "WAMRModule.h"

// Forward declarations for WAMR types
typedef struct WASMModuleInstanceCommon* wasm_module_inst_t;
typedef struct WASMExecEnv* wasm_exec_env_t;

namespace hgl::wasm
{
    /**
     * WAMR执行上下文实现
     */
    class WAMRContext : public IWasmContext
    {
    private:
        std::shared_ptr<IWasmModule> module_;
        wasm_module_inst_t module_inst_;
        wasm_exec_env_t exec_env_;
        std::string error_message_;
        uint32_t stack_size_;
        uint32_t heap_size_;

    public:
        WAMRContext();
        virtual ~WAMRContext();

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
        VMType GetVMType() const override { return VMType::WAMR; }

        void SetStackSize(uint32_t size) { stack_size_ = size; }
        void SetHeapSize(uint32_t size) { heap_size_ = size; }

    private:
        void SetError(const std::string& msg);
        uint8_t ConvertValueType(ValueType type);
    };

}//namespace hgl::wasm
