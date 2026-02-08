# WASI-SDK 快速参考

## 目录结构

```
CMScript/
├── docs/
│   └── WASI_SDK_GUIDE.md          # 完整使用指南
├── scripts/
│   ├── download_wasi_sdk.bat      # Windows 下载脚本
│   ├── download_wasi_sdk.sh       # Linux/Mac 下载脚本
│   ├── compile_to_wasm.bat        # Windows 编译到 .wasm
│   ├── compile_to_wasm.sh         # Linux/Mac 编译到 .wasm
│   ├── compile_to_aot.bat         # Windows 编译到 .aot
│   └── compile_to_aot.sh          # Linux/Mac 编译到 .aot
├── examples/wasm/
│   ├── README.md                  # 示例说明
│   ├── add.c                      # 基本算术示例
│   ├── math_lib.c                 # 数学库示例
│   └── string_ops.c               # 字符串操作示例
└── third_party/wasi/              # WASI-SDK 安装目录
    └── bin/
        ├── clang.exe              # C 编译器
        └── clang++.exe            # C++ 编译器
```

## 快速开始

### 1. 下载 WASI-SDK（如果还没有）

```bash
# Windows
cd scripts
download_wasi_sdk.bat

# Linux/macOS
cd scripts
chmod +x download_wasi_sdk.sh
./download_wasi_sdk.sh
```

### 2. 编译 C 代码到 .wasm

```bash
# Windows
scripts\compile_to_wasm.bat examples\wasm\add.c build\add.wasm

# Linux/macOS
scripts/compile_to_wasm.sh examples/wasm/add.c build/add.wasm
```

### 3. 编译 .wasm 到 .aot（可选，提高性能）

```bash
# Windows
scripts\compile_to_aot.bat build\add.wasm build\add.aot

# Linux/macOS
scripts/compile_to_aot.sh build/add.wasm build/add.aot
```

## 常用命令

### 基本编译
```bash
# 编译 C 文件
scripts/compile_to_wasm.sh input.c output.wasm

# 编译 C++ 文件
scripts/compile_to_wasm.sh input.cpp output.wasm
```

### 带优化的编译
```bash
# -O2 优化
scripts/compile_to_wasm.sh input.c output.wasm -O2

# -Oz 大小优化
scripts/compile_to_wasm.sh input.c output.wasm -Oz

# -O3 激进优化
scripts/compile_to_wasm.sh input.c output.wasm -O3
```

### 导出函数
```bash
# 导出特定函数
scripts/compile_to_wasm.sh lib.c lib.wasm "-Wl,--export=my_function"

# 导出所有函数
scripts/compile_to_wasm.sh lib.c lib.wasm "-Wl,--export-all"
```

### AOT 编译选项
```bash
# 基本 AOT 编译
scripts/compile_to_aot.sh input.wasm output.aot

# 指定优化级别
scripts/compile_to_aot.sh input.wasm output.aot --opt-level=3

# 指定目标架构
scripts/compile_to_aot.sh input.wasm output.aot --target=x86_64

# 启用 SIMD
scripts/compile_to_aot.sh input.wasm output.aot --enable-simd
```

## 示例代码片段

### 简单函数

```c
// add.c
__attribute__((export_name("add")))
int add(int a, int b) {
    return a + b;
}
```

### 数学函数

```c
// math.c
#include <math.h>

__attribute__((export_name("circle_area")))
double circle_area(double radius) {
    return 3.14159265359 * radius * radius;
}
```

### 在 CMScript 中使用

```cpp
#include <hgl/wasm/WasmVM.h>

// 初始化
hgl::wasm::InitializeVM(hgl::wasm::VMType::WAMR);

// 加载模块
auto module = hgl::wasm::CreateWasmModule(hgl::wasm::VMType::WAMR);
module->LoadFromFile("build/add.wasm");

// 创建上下文
auto context = hgl::wasm::CreateWasmContext(hgl::wasm::VMType::WAMR);
context->Instantiate(module);

// 调用函数
hgl::wasm::Value args[2] = {
    hgl::wasm::Value::MakeI32(10),
    hgl::wasm::Value::MakeI32(32)
};
hgl::wasm::Value result[1];
context->CallFunction("add", args, 2, result, 1);

printf("Result: %d\n", result[0].data.i32);

// 清理
hgl::wasm::CleanupVM(hgl::wasm::VMType::WAMR);
```

## 常见问题

### Q: 如何检查 WASI-SDK 是否安装？
```bash
# Windows
third_party\wasi\bin\clang.exe --version

# Linux/macOS
third_party/wasi/bin/clang --version
```

### Q: 如何减小 .wasm 文件大小？
使用 `-Oz` 优化并去除调试信息：
```bash
scripts/compile_to_wasm.sh input.c output.wasm "-Oz -Wl,--strip-all"
```

### Q: AOT 编译器在哪里？
需要先构建 WAMR 的 wamrc 工具：
```bash
git submodule update --init --recursive
cd third_party/wasm-micro-runtime/wamr-compiler
mkdir build && cd build
cmake ..
cmake --build .
```

### Q: 如何查看 .wasm 文件内容？
使用 wasm-objdump（需要单独安装）：
```bash
wasm-objdump -x output.wasm
```

## 进阶用法

### 编译静态库
```bash
# 编译多个源文件
scripts/compile_to_wasm.sh "file1.c file2.c file3.c" output.wasm
```

### 使用自定义标志
```bash
# 添加包含路径
scripts/compile_to_wasm.sh input.c output.wasm "-I./include"

# 定义宏
scripts/compile_to_wasm.sh input.c output.wasm "-DDEBUG=1"

# 链接数学库
scripts/compile_to_wasm.sh input.c output.wasm "-lm"
```

### 性能对比

| 格式 | 加载速度 | 执行速度 | 文件大小 | 可移植性 |
|------|---------|---------|---------|---------|
| .wasm | 中等 | 较快 | 较小 | 跨平台 |
| .aot | 快 | 最快 | 较大 | 平台相关 |

**建议**：
- 开发调试：使用 .wasm
- 生产部署：使用 .aot（如果性能重要）

## 更多资源

- [完整使用指南](../docs/WASI_SDK_GUIDE.md)
- [WASM VM 文档](../src/WasmVM/README.md)
- [示例代码](../examples/wasm/)
- [WASI-SDK 官方文档](https://github.com/WebAssembly/wasi-sdk)
- [WAMR 文档](https://github.com/bytecodealliance/wasm-micro-runtime)

## 故障排除

### 问题：找不到 clang
**解决**：确保 WASI-SDK 已正确安装在 `third_party/wasi` 目录

### 问题：编译时提示找不到头文件
**解决**：WASI-SDK 应该包含完整的 sysroot，确保下载的是完整版本

### 问题：AOT 编译失败
**解决**：
1. 确保 wamrc 已正确构建
2. 检查目标架构是否匹配
3. 验证 .wasm 文件是否有效

### 问题：.wasm 文件太大
**解决**：
1. 使用 `-Oz` 优化
2. 使用 `-Wl,--strip-all` 去除调试信息
3. 只导出必要的函数
4. 考虑使用 wasm-opt 进一步优化

---

**提示**：更多详细信息，请查看 [完整使用指南](../docs/WASI_SDK_GUIDE.md)
