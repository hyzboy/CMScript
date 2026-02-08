#include "WasmEdgeContext.h"
#include "WasmEdgeModule.h"
#include <wasmedge/wasmedge.h>
#include <cstring>

namespace hgl::wasm
{
    WasmEdgeContext::WasmEdgeContext()
        : vm_context_(nullptr)
        , module_inst_(nullptr)
    {
    }

    WasmEdgeContext::~WasmEdgeContext()
    {
        if (vm_context_)
        {
            WasmEdge_VMDelete(vm_context_);
            vm_context_ = nullptr;
        }
        // module_inst_ is owned by vm_context_, no need to delete separately
        module_inst_ = nullptr;
    }

    bool WasmEdgeContext::Instantiate(std::shared_ptr<IWasmModule> module)
    {
        if (!module || !module->IsValid())
        {
            SetError("Invalid module");
            return false;
        }

        module_ = module;

        // Get WasmEdge module
        auto wasmedge_module = std::dynamic_pointer_cast<WasmEdgeModule>(module);
        if (!wasmedge_module)
        {
            SetError("Module is not a WasmEdge module");
            return false;
        }

        // Create VM context
        if (vm_context_)
        {
            WasmEdge_VMDelete(vm_context_);
        }

        vm_context_ = WasmEdge_VMCreate(wasmedge_module->GetConfig(), nullptr);
        if (!vm_context_)
        {
            SetError("Failed to create VM context");
            return false;
        }

        // Register and instantiate the module
        WasmEdge_Result result = WasmEdge_VMRegisterModuleFromASTModule(
            vm_context_, "", wasmedge_module->GetNativeModule());
        
        if (!WasmEdge_ResultOK(result))
        {
            SetError("Failed to register module");
            return false;
        }

        // Get the module instance
        module_inst_ = WasmEdge_VMGetRegisteredModule(vm_context_, WasmEdge_StringCreateByCString(""));

        return true;
    }

    bool WasmEdgeContext::CallFunction(
        const char* func_name,
        const Value* args,
        size_t argc,
        Value* results,
        size_t result_count)
    {
        if (!vm_context_)
        {
            SetError("VM not instantiated");
            return false;
        }

        if (!func_name)
        {
            SetError("Invalid function name");
            return false;
        }

        // Convert arguments to WasmEdge format
        std::vector<WasmEdge_Value> wasmedge_args;
        for (size_t i = 0; i < argc; ++i)
        {
            WasmEdge_Value val;
            switch (args[i].type)
            {
                case ValueType::I32:
                    val = WasmEdge_ValueGenI32(args[i].data.i32);
                    break;
                case ValueType::I64:
                    val = WasmEdge_ValueGenI64(args[i].data.i64);
                    break;
                case ValueType::F32:
                    val = WasmEdge_ValueGenF32(args[i].data.f32);
                    break;
                case ValueType::F64:
                    val = WasmEdge_ValueGenF64(args[i].data.f64);
                    break;
                default:
                    SetError("Unsupported argument type");
                    return false;
            }
            wasmedge_args.push_back(val);
        }

        // Prepare result buffer
        std::vector<WasmEdge_Value> wasmedge_results(result_count);

        // Call function
        WasmEdge_String func_name_str = WasmEdge_StringCreateByCString(func_name);
        WasmEdge_Result result = WasmEdge_VMExecute(
            vm_context_,
            func_name_str,
            wasmedge_args.empty() ? nullptr : wasmedge_args.data(),
            static_cast<uint32_t>(wasmedge_args.size()),
            wasmedge_results.empty() ? nullptr : wasmedge_results.data(),
            static_cast<uint32_t>(wasmedge_results.size()));
        
        WasmEdge_StringDelete(func_name_str);

        if (!WasmEdge_ResultOK(result))
        {
            SetError("Function execution failed");
            return false;
        }

        // Convert results back
        for (size_t i = 0; i < result_count && i < wasmedge_results.size(); ++i)
        {
            WasmEdge_ValType type = WasmEdge_ValueGetType(wasmedge_results[i]);
            switch (type)
            {
                case WasmEdge_ValType_I32:
                    results[i].type = ValueType::I32;
                    results[i].data.i32 = WasmEdge_ValueGetI32(wasmedge_results[i]);
                    break;
                case WasmEdge_ValType_I64:
                    results[i].type = ValueType::I64;
                    results[i].data.i64 = WasmEdge_ValueGetI64(wasmedge_results[i]);
                    break;
                case WasmEdge_ValType_F32:
                    results[i].type = ValueType::F32;
                    results[i].data.f32 = WasmEdge_ValueGetF32(wasmedge_results[i]);
                    break;
                case WasmEdge_ValType_F64:
                    results[i].type = ValueType::F64;
                    results[i].data.f64 = WasmEdge_ValueGetF64(wasmedge_results[i]);
                    break;
                default:
                    results[i].type = ValueType::I32;
                    results[i].data.i32 = 0;
                    break;
            }
        }

        return true;
    }

    bool WasmEdgeContext::CallFunction(const char* func_name)
    {
        return CallFunction(func_name, nullptr, 0, nullptr, 0);
    }

    bool WasmEdgeContext::CallFunctionI32(const char* func_name, int32_t arg, int32_t& result)
    {
        Value args[1];
        args[0] = Value::MakeI32(arg);
        
        Value results[1];
        if (!CallFunction(func_name, args, 1, results, 1))
            return false;

        result = results[0].data.i32;
        return true;
    }

    uint8_t* WasmEdgeContext::GetMemory(const char* memory_name)
    {
        if (!vm_context_)
        {
            SetError("VM not instantiated");
            return nullptr;
        }

        WasmEdge_String mem_name = WasmEdge_StringCreateByCString(memory_name ? memory_name : "memory");
        WasmEdge_MemoryInstanceContext* mem = WasmEdge_VMGetMemoryInstanceContext(vm_context_, mem_name);
        WasmEdge_StringDelete(mem_name);

        if (!mem)
        {
            SetError("Memory not found");
            return nullptr;
        }

        return WasmEdge_MemoryInstanceGetPointer(mem, 0, 0);
    }

    size_t WasmEdgeContext::GetMemorySize(const char* memory_name)
    {
        if (!vm_context_)
        {
            SetError("VM not instantiated");
            return 0;
        }

        WasmEdge_String mem_name = WasmEdge_StringCreateByCString(memory_name ? memory_name : "memory");
        WasmEdge_MemoryInstanceContext* mem = WasmEdge_VMGetMemoryInstanceContext(vm_context_, mem_name);
        WasmEdge_StringDelete(mem_name);

        if (!mem)
        {
            SetError("Memory not found");
            return 0;
        }

        return WasmEdge_MemoryInstanceGetPageSize(mem) * 65536; // Page size is 64KB
    }

    void WasmEdgeContext::SetError(const std::string& msg)
    {
        error_message_ = msg;
    }

    enum WasmEdge_ValType WasmEdgeContext::ConvertValueType(ValueType type)
    {
        switch (type)
        {
            case ValueType::I32: return WasmEdge_ValType_I32;
            case ValueType::I64: return WasmEdge_ValType_I64;
            case ValueType::F32: return WasmEdge_ValType_F32;
            case ValueType::F64: return WasmEdge_ValType_F64;
            default: return WasmEdge_ValType_I32;
        }
    }

}//namespace hgl::wasm
