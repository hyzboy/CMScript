#pragma once

#include <cstdint>
#include <vector>
#include <string>

/**
 * WASM虚拟机共用要素
 */
namespace hgl::wasm
{
    /**
     * WASM虚拟机类型
     */
    enum class VMType
    {
        WAMR,       // WebAssembly Micro Runtime
        WasmEdge    // WasmEdge Runtime
    };

    /**
     * WASM值类型
     */
    enum class ValueType
    {
        I32,        // 32位整数
        I64,        // 64位整数
        F32,        // 32位浮点数
        F64,        // 64位浮点数
        V128,       // 128位向量
        FuncRef,    // 函数引用
        ExternRef   // 外部引用
    };

    /**
     * WASM值
     */
    struct Value
    {
        ValueType type;
        union
        {
            int32_t i32;
            int64_t i64;
            float f32;
            double f64;
            void* ref;
        } data;

        Value() : type(ValueType::I32) { data.i32 = 0; }
        
        static Value MakeI32(int32_t val)
        {
            Value v;
            v.type = ValueType::I32;
            v.data.i32 = val;
            return v;
        }

        static Value MakeI64(int64_t val)
        {
            Value v;
            v.type = ValueType::I64;
            v.data.i64 = val;
            return v;
        }

        static Value MakeF32(float val)
        {
            Value v;
            v.type = ValueType::F32;
            v.data.f32 = val;
            return v;
        }

        static Value MakeF64(double val)
        {
            Value v;
            v.type = ValueType::F64;
            v.data.f64 = val;
            return v;
        }
    };

    /**
     * 主机函数签名
     */
    using HostFunction = bool (*)(void* env, const Value* args, size_t argc, Value* results, size_t result_count);

}//namespace hgl::wasm
