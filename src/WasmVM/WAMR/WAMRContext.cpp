#include "WAMRContext.h"
#include "WAMRModule.h"
#include <wasm_export.h>
#include <cstring>

namespace hgl::wasm
{
    WAMRContext::WAMRContext()
        : module_inst_(nullptr)
        , exec_env_(nullptr)
        , stack_size_(16 * 1024)  // 16KB default stack
        , heap_size_(16 * 1024)   // 16KB default heap
    {
    }

    WAMRContext::~WAMRContext()
    {
        if (exec_env_)
        {
            wasm_runtime_destroy_exec_env(exec_env_);
            exec_env_ = nullptr;
        }

        if (module_inst_)
        {
            wasm_runtime_deinstantiate(module_inst_);
            module_inst_ = nullptr;
        }
    }

    bool WAMRContext::Instantiate(std::shared_ptr<IWasmModule> module)
    {
        if (!module || !module->IsValid())
        {
            SetError("Invalid module");
            return false;
        }

        module_ = module;

        // Get WAMR module
        auto wamr_module = std::dynamic_pointer_cast<WAMRModule>(module);
        if (!wamr_module)
        {
            SetError("Module is not a WAMR module");
            return false;
        }

        char error_buf[128];
        module_inst_ = wasm_runtime_instantiate(
            wamr_module->GetNativeModule(),
            stack_size_,
            heap_size_,
            error_buf,
            sizeof(error_buf));

        if (!module_inst_)
        {
            SetError("Failed to instantiate module: " + std::string(error_buf));
            return false;
        }

        exec_env_ = wasm_runtime_create_exec_env(module_inst_, stack_size_);
        if (!exec_env_)
        {
            SetError("Failed to create execution environment");
            wasm_runtime_deinstantiate(module_inst_);
            module_inst_ = nullptr;
            return false;
        }

        return true;
    }

    bool WAMRContext::CallFunction(
        const char* func_name,
        const Value* args,
        size_t argc,
        Value* results,
        size_t result_count)
    {
        if (!module_inst_ || !exec_env_)
        {
            SetError("Module not instantiated");
            return false;
        }

        if (!func_name)
        {
            SetError("Invalid function name");
            return false;
        }

        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, func_name, nullptr);
        if (!func)
        {
            SetError("Function not found: " + std::string(func_name));
            return false;
        }

        // Convert arguments to WAMR format
        std::vector<uint32_t> wamr_args;
        for (size_t i = 0; i < argc; ++i)
        {
            switch (args[i].type)
            {
                case ValueType::I32:
                    wamr_args.push_back(static_cast<uint32_t>(args[i].data.i32));
                    break;
                case ValueType::I64:
                    wamr_args.push_back(static_cast<uint32_t>(args[i].data.i64 & 0xFFFFFFFF));
                    wamr_args.push_back(static_cast<uint32_t>((args[i].data.i64 >> 32) & 0xFFFFFFFF));
                    break;
                case ValueType::F32:
                    {
                        uint32_t val;
                        std::memcpy(&val, &args[i].data.f32, sizeof(uint32_t));
                        wamr_args.push_back(val);
                    }
                    break;
                case ValueType::F64:
                    {
                        uint64_t val;
                        std::memcpy(&val, &args[i].data.f64, sizeof(uint64_t));
                        wamr_args.push_back(static_cast<uint32_t>(val & 0xFFFFFFFF));
                        wamr_args.push_back(static_cast<uint32_t>((val >> 32) & 0xFFFFFFFF));
                    }
                    break;
                default:
                    SetError("Unsupported argument type");
                    return false;
            }
        }

        // Call function
        if (!wasm_runtime_call_wasm(exec_env_, func, 
                                    static_cast<uint32_t>(wamr_args.size()),
                                    wamr_args.empty() ? nullptr : wamr_args.data()))
        {
            const char* exception = wasm_runtime_get_exception(module_inst_);
            SetError("Function execution failed: " + std::string(exception ? exception : "Unknown error"));
            return false;
        }

        // Get results
        if (result_count > 0 && results)
        {
            // WAMR returns results in the same buffer
            // For simplicity, we assume single I32 result
            if (result_count == 1 && !wamr_args.empty())
            {
                results[0].type = ValueType::I32;
                results[0].data.i32 = static_cast<int32_t>(wamr_args[0]);
            }
        }

        return true;
    }

    bool WAMRContext::CallFunction(const char* func_name)
    {
        return CallFunction(func_name, nullptr, 0, nullptr, 0);
    }

    bool WAMRContext::CallFunctionI32(const char* func_name, int32_t arg, int32_t& result)
    {
        Value args[1];
        args[0] = Value::MakeI32(arg);
        
        Value results[1];
        if (!CallFunction(func_name, args, 1, results, 1))
            return false;

        result = results[0].data.i32;
        return true;
    }

    uint8_t* WAMRContext::GetMemory(const char* memory_name)
    {
        if (!module_inst_)
        {
            SetError("Module not instantiated");
            return nullptr;
        }

        return wasm_runtime_get_memory_data(module_inst_);
    }

    size_t WAMRContext::GetMemorySize(const char* memory_name)
    {
        if (!module_inst_)
        {
            SetError("Module not instantiated");
            return 0;
        }

        return wasm_runtime_get_memory_data_size(module_inst_);
    }

    void WAMRContext::SetError(const std::string& msg)
    {
        error_message_ = msg;
    }

    uint8_t WAMRContext::ConvertValueType(ValueType type)
    {
        switch (type)
        {
            case ValueType::I32: return WASM_I32;
            case ValueType::I64: return WASM_I64;
            case ValueType::F32: return WASM_F32;
            case ValueType::F64: return WASM_F64;
            default: return WASM_I32;
        }
    }

}//namespace hgl::wasm
