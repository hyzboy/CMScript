#include "WAMRModule.h"
#include <wasm_export.h>
#include <fstream>
#include <cstring>

namespace hgl::wasm
{
    WAMRModule::WAMRModule()
        : module_(nullptr)
    {
    }

    WAMRModule::~WAMRModule()
    {
        if (module_)
        {
            wasm_runtime_unload(module_);
            module_ = nullptr;
        }
    }

    bool WAMRModule::LoadFromFile(const char* filename)
    {
        if (!filename)
        {
            SetError("Invalid filename");
            return false;
        }

        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            SetError("Failed to open file: " + std::string(filename));
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        binary_data_.resize(size);
        if (!file.read(reinterpret_cast<char*>(binary_data_.data()), size))
        {
            SetError("Failed to read file: " + std::string(filename));
            return false;
        }

        return LoadFromMemory(binary_data_.data(), binary_data_.size());
    }

    bool WAMRModule::LoadFromMemory(const uint8_t* data, size_t size)
    {
        if (!data || size == 0)
        {
            SetError("Invalid data or size");
            return false;
        }

        // Clean up existing module
        if (module_)
        {
            wasm_runtime_unload(module_);
            module_ = nullptr;
        }

        // Store binary data
        binary_data_.assign(data, data + size);

        char error_buf[128];
        module_ = wasm_runtime_load(binary_data_.data(), binary_data_.size(),
                                    error_buf, sizeof(error_buf));

        if (!module_)
        {
            SetError("Failed to load WASM module: " + std::string(error_buf));
            return false;
        }

        return true;
    }

    bool WAMRModule::RegisterHostFunction(
        const char* module_name,
        const char* func_name,
        HostFunction func,
        void* env,
        const std::vector<ValueType>& param_types,
        const std::vector<ValueType>& result_types)
    {
        // WAMR host function registration would be done before module loading
        // This is a simplified implementation
        SetError("Host function registration not yet implemented for WAMR");
        return false;
    }

    bool WAMRModule::IsValid() const
    {
        return module_ != nullptr;
    }

    void WAMRModule::SetError(const std::string& msg)
    {
        error_message_ = msg;
    }

}//namespace hgl::wasm
