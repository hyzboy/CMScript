# WebAssembly Examples

本目录包含使用 WASI-SDK 编译 WebAssembly 的示例代码。

## 示例文件

### 1. add.c - 基本算术运算
简单的加减乘除函数示例。

编译：
```bash
scripts/compile_to_wasm.sh examples/wasm/add.c build/add.wasm
```

### 2. math_lib.c - 数学库
包含各种数学计算函数：圆形面积、勾股定理、阶乘、斐波那契等。

编译：
```bash
scripts/compile_to_wasm.sh examples/wasm/math_lib.c build/math_lib.wasm -O2
```

### 3. string_ops.c - 字符串操作
字符串长度、比较、复制、连接等操作。

编译：
```bash
scripts/compile_to_wasm.sh examples/wasm/string_ops.c build/string_ops.wasm
```

## 使用方法

### 步骤 1: 下载 WASI-SDK

```bash
# Windows
scripts\download_wasi_sdk.bat

# Linux/macOS
scripts/download_wasi_sdk.sh
```

### 步骤 2: 编译为 .wasm

```bash
# Windows
scripts\compile_to_wasm.bat examples\wasm\add.c build\add.wasm

# Linux/macOS
scripts/compile_to_wasm.sh examples/wasm/add.c build/add.wasm
```

### 步骤 3: （可选）编译为 .aot

```bash
# Windows
scripts\compile_to_aot.bat build\add.wasm build\add.aot

# Linux/macOS
scripts/compile_to_aot.sh build/add.wasm build/add.aot
```

### 步骤 4: 在 CMScript 中使用

```cpp
#include <hgl/wasm/WasmVM.h>

// 初始化 WAMR
hgl::wasm::InitializeVM(hgl::wasm::VMType::WAMR);

// 加载模块
auto module = hgl::wasm::CreateWasmModule(hgl::wasm::VMType::WAMR);
module->LoadFromFile("build/add.wasm");

// 创建执行上下文
auto context = hgl::wasm::CreateWasmContext(hgl::wasm::VMType::WAMR);
context->Instantiate(module);

// 调用函数
hgl::wasm::Value args[2] = {
    hgl::wasm::Value::MakeI32(10),
    hgl::wasm::Value::MakeI32(32)
};
hgl::wasm::Value result[1];
context->CallFunction("add", args, 2, result, 1);

printf("Result: %d\n", result[0].data.i32);  // 输出: 42

// 清理
hgl::wasm::CleanupVM(hgl::wasm::VMType::WAMR);
```

## 编译选项

### 基本优化
```bash
scripts/compile_to_wasm.sh input.c output.wasm -O2
```

### 导出特定函数
```bash
scripts/compile_to_wasm.sh input.c output.wasm "-Wl,--export=my_function"
```

### 大小优化
```bash
scripts/compile_to_wasm.sh input.c output.wasm -Oz
```

### 调试信息
```bash
scripts/compile_to_wasm.sh input.c output.wasm -g
```

## 更多信息

详细文档请参见：
- [WASI-SDK 使用指南](../../docs/WASI_SDK_GUIDE.md)
- [WASM VM 文档](../../src/WasmVM/README.md)
