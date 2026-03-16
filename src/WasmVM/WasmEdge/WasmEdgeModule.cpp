#include "WasmEdgeModule.h"
#include <wasmedge/wasmedge.h>
#include <fstream>
#include <cstring>

namespace hgl::wasm
{
    WasmEdgeModule::WasmEdgeModule()
        : config_(nullptr)
        , loader_(nullptr)
        , validator_(nullptr)
        , ast_module_(nullptr)
    {
        config_ = WasmEdge_ConfigureCreate();
        loader_ = WasmEdge_LoaderCreate(config_);
        validator_ = WasmEdge_ValidatorCreate(config_);
    }

    WasmEdgeModule::~WasmEdgeModule()
    {
        if (ast_module_)
        {
            WasmEdge_ASTModuleDelete(ast_module_);
            ast_module_ = nullptr;
        }

        if (validator_)
        {
            WasmEdge_ValidatorDelete(validator_);
            validator_ = nullptr;
        }

        if (loader_)
        {
            WasmEdge_LoaderDelete(loader_);
            loader_ = nullptr;
        }

        if (config_)
        {
            WasmEdge_ConfigureDelete(config_);
            config_ = nullptr;
        }
    }

    bool WasmEdgeModule::LoadFromFile(const char* filename)
    {
        if (!filename)
        {
            SetError("Invalid filename");
            return false;
        }

        if (!loader_)
        {
            SetError("Loader not initialized");
            return false;
        }

        // Clean up existing module
        if (ast_module_)
        {
            WasmEdge_ASTModuleDelete(ast_module_);
            ast_module_ = nullptr;
        }

        WasmEdge_Result result = WasmEdge_LoaderParseFromFile(loader_, &ast_module_, filename);
        if (!WasmEdge_ResultOK(result))
        {
            SetError("Failed to load WASM file: " + std::string(filename));
            return false;
        }

        // Validate the module
        result = WasmEdge_ValidatorValidate(validator_, ast_module_);
        if (!WasmEdge_ResultOK(result))
        {
            SetError("Module validation failed");
            WasmEdge_ASTModuleDelete(ast_module_);
            ast_module_ = nullptr;
            return false;
        }

        return true;
    }

    bool WasmEdgeModule::LoadFromMemory(const uint8_t* data, size_t size)
    {
        if (!data || size == 0)
        {
            SetError("Invalid data or size");
            return false;
        }

        if (!loader_)
        {
            SetError("Loader not initialized");
            return false;
        }

        // Clean up existing module
        if (ast_module_)
        {
            WasmEdge_ASTModuleDelete(ast_module_);
            ast_module_ = nullptr;
        }

        // Store binary data
        binary_data_.assign(data, data + size);

        WasmEdge_Result result = WasmEdge_LoaderParseFromBuffer(loader_, &ast_module_, 
                                                                 binary_data_.data(), binary_data_.size());
        if (!WasmEdge_ResultOK(result))
        {
            SetError("Failed to load WASM from memory");
            return false;
        }

        // Validate the module
        result = WasmEdge_ValidatorValidate(validator_, ast_module_);
        if (!WasmEdge_ResultOK(result))
        {
            SetError("Module validation failed");
            WasmEdge_ASTModuleDelete(ast_module_);
            ast_module_ = nullptr;
            return false;
        }

        return true;
    }

    bool WasmEdgeModule::RegisterHostFunction(
        const char* module_name,
        const char* func_name,
        HostFunction func,
        void* env,
        const std::vector<ValueType>& param_types,
        const std::vector<ValueType>& result_types)
    {
        // WasmEdge host function registration would be done through import object
        // This is a simplified implementation
        SetError("Host function registration not yet implemented for WasmEdge");
        return false;
    }

    bool WasmEdgeModule::IsValid() const
    {
        return ast_module_ != nullptr;
    }

    void WasmEdgeModule::SetError(const std::string& msg)
    {
        error_message_ = msg;
    }

}//namespace hgl::wasm
