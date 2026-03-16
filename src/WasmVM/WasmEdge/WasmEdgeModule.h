#pragma once

#include <hgl/wasm/WasmModule.h>
#include <string>
#include <vector>

// Forward declarations for WasmEdge C API types
struct WasmEdge_ConfigureContext;
struct WasmEdge_ASTModuleContext;
struct WasmEdge_LoaderContext;
struct WasmEdge_ValidatorContext;

namespace hgl::wasm
{
    /**
     * WasmEdge模块实现
     */
    class WasmEdgeModule : public IWasmModule
    {
    private:
        WasmEdge_ConfigureContext* config_;
        WasmEdge_LoaderContext* loader_;
        WasmEdge_ValidatorContext* validator_;
        WasmEdge_ASTModuleContext* ast_module_;
        std::string error_message_;
        std::vector<uint8_t> binary_data_;

    public:
        WasmEdgeModule();
        virtual ~WasmEdgeModule();

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
        VMType GetVMType() const override { return VMType::WasmEdge; }
        std::string GetErrorMessage() const override { return error_message_; }

        WasmEdge_ASTModuleContext* GetNativeModule() const { return ast_module_; }
        WasmEdge_ConfigureContext* GetConfig() const { return config_; }

    private:
        void SetError(const std::string& msg);
    };

}//namespace hgl::wasm
