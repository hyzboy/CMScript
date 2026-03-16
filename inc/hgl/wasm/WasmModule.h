#pragma once

#include <hgl/wasm/VM.h>
#include <string>
#include <vector>
#include <memory>

namespace hgl::wasm
{
    /**
     * WASM模块接口
     * 用于加载和管理WebAssembly模块
     */
    class IWasmModule
    {
    public:
        virtual ~IWasmModule() = default;

        /**
         * 从文件加载WASM模块
         * @param filename WASM文件路径
         * @return 成功返回true，失败返回false
         */
        virtual bool LoadFromFile(const char* filename) = 0;

        /**
         * 从内存加载WASM模块
         * @param data WASM字节码数据
         * @param size 数据大小
         * @return 成功返回true，失败返回false
         */
        virtual bool LoadFromMemory(const uint8_t* data, size_t size) = 0;

        /**
         * 注册主机函数
         * @param module_name 模块名称
         * @param func_name 函数名称
         * @param func 函数指针
         * @param env 环境参数
         * @param param_types 参数类型列表
         * @param result_types 返回值类型列表
         * @return 成功返回true，失败返回false
         */
        virtual bool RegisterHostFunction(
            const char* module_name,
            const char* func_name,
            HostFunction func,
            void* env,
            const std::vector<ValueType>& param_types,
            const std::vector<ValueType>& result_types) = 0;

        /**
         * 验证模块是否有效
         * @return 有效返回true，无效返回false
         */
        virtual bool IsValid() const = 0;

        /**
         * 获取虚拟机类型
         * @return 虚拟机类型
         */
        virtual VMType GetVMType() const = 0;

        /**
         * 获取错误信息
         * @return 错误信息字符串
         */
        virtual std::string GetErrorMessage() const = 0;
    };

    /**
     * 创建WASM模块实例
     * @param type 虚拟机类型
     * @return 模块实例智能指针
     */
    std::shared_ptr<IWasmModule> CreateWasmModule(VMType type);

}//namespace hgl::wasm
