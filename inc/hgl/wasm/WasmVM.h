#pragma once

#include <hgl/wasm/VM.h>
#include <hgl/wasm/WasmModule.h>
#include <hgl/wasm/WasmContext.h>

/**
 * WASM虚拟机主头文件
 * 包含所有必要的WASM VM接口
 */
namespace hgl::wasm
{
    /**
     * 初始化WASM虚拟机
     * @param type 虚拟机类型
     * @return 成功返回true，失败返回false
     */
    bool InitializeVM(VMType type);

    /**
     * 清理WASM虚拟机
     * @param type 虚拟机类型
     */
    void CleanupVM(VMType type);

}//namespace hgl::wasm
