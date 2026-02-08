#pragma once

#include <hgl/wasm/WasmModule.h>
#include <string>
#include <vector>

// Forward declarations for WAMR types
typedef struct WASMModuleCommon* wasm_module_t;
typedef struct WASMModuleInstanceCommon* wasm_module_inst_t;
typedef struct WASMFunctionInstanceCommon* wasm_function_inst_t;

namespace hgl::wasm
{
    /**
     * WAMR模块实现
     */
    class WAMRModule : public IWasmModule
    {
    private:
        wasm_module_t module_;
        std::string error_message_;
        std::vector<uint8_t> binary_data_;

    public:
        WAMRModule();
        virtual ~WAMRModule();

        bool LoadFromFile(const char* filename) override;
        bool LoadFromMemory(const uint8_t* data, size_t size) override;
        
        bool RegisterHostFunction(
            const char* module_name,
            const char* func_name,
            HostFunction func,
            void* env,
            const std::vector<ValueType>& param_types,
            const std::vector<ValueType>& result_types) override;

        bool IsValid() const override;
        VMType GetVMType() const override { return VMType::WAMR; }
        std::string GetErrorMessage() const override { return error_message_; }

        wasm_module_t GetNativeModule() const { return module_; }

    private:
        void SetError(const std::string& msg);
    };

}//namespace hgl::wasm
