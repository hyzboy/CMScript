#pragma once

#include <hgl/wasm/WasmModule.h>
#include <string>
#include <vector>

namespace hgl::wasm
{
    /**
     * WASM执行上下文接口
     * 用于执行WebAssembly函数
     */
    class IWasmContext
    {
    public:
        virtual ~IWasmContext() = default;

        /**
         * 实例化模块
         * @param module WASM模块
         * @return 成功返回true，失败返回false
         */
        virtual bool Instantiate(std::shared_ptr<IWasmModule> module) = 0;

        /**
         * 调用导出函数
         * @param func_name 函数名称
         * @param args 参数数组
         * @param argc 参数个数
         * @param results 结果数组
         * @param result_count 结果个数
         * @return 成功返回true，失败返回false
         */
        virtual bool CallFunction(
            const char* func_name,
            const Value* args,
            size_t argc,
            Value* results,
            size_t result_count) = 0;

        /**
         * 便捷调用函数（无参数无返回值）
         * @param func_name 函数名称
         * @return 成功返回true，失败返回false
         */
        virtual bool CallFunction(const char* func_name) = 0;

        /**
         * 便捷调用函数（单个I32参数，单个I32返回值）
         * @param func_name 函数名称
         * @param arg 参数
         * @param result 结果
         * @return 成功返回true，失败返回false
         */
        virtual bool CallFunctionI32(const char* func_name, int32_t arg, int32_t& result) = 0;

        /**
         * 获取导出的内存
         * @param memory_name 内存名称（默认为"memory"）
         * @return 内存指针，失败返回nullptr
         */
        virtual uint8_t* GetMemory(const char* memory_name = "memory") = 0;

        /**
         * 获取内存大小
         * @param memory_name 内存名称（默认为"memory"）
         * @return 内存大小（字节）
         */
        virtual size_t GetMemorySize(const char* memory_name = "memory") = 0;

        /**
         * 获取错误信息
         * @return 错误信息字符串
         */
        virtual std::string GetErrorMessage() const = 0;

        /**
         * 获取虚拟机类型
         * @return 虚拟机类型
         */
        virtual VMType GetVMType() const = 0;
    };

    /**
     * 创建WASM执行上下文
     * @param type 虚拟机类型
     * @return 上下文实例智能指针
     */
    std::shared_ptr<IWasmContext> CreateWasmContext(VMType type);

}//namespace hgl::wasm
