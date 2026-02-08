# WASI-SDK 使用指南

本指南详细介绍如何使用 WASI-SDK 将 C/C++ 代码编译为 WebAssembly (.wasm) 格式，以及如何使用 WAMR 编译器将 .wasm 编译为 AOT (.aot) 格式。

## 目录

- [WASI-SDK 简介](#wasi-sdk-简介)
- [下载和安装](#下载和安装)
- [编译 C/C++ 到 .wasm](#编译-cc-到-wasm)
- [编译 .wasm 到 .aot](#编译-wasm-到-aot)
- [自动化脚本](#自动化脚本)
- [示例](#示例)
- [常见问题](#常见问题)

## WASI-SDK 简介

WASI-SDK 是一个基于 Clang/LLVM 的工具链，用于将 C/C++ 代码编译为 WebAssembly 格式。它包含：

- **clang/clang++**: C/C++ 编译器
- **wasm-ld**: WebAssembly 链接器
- **llvm-ar**: 静态库工具
- **WASI libc**: WebAssembly 系统接口标准库

### 支持的目标平台

- `wasm32-wasi`: 标准 WASI 目标
- `wasm32-wasi-threads`: 支持线程的 WASI

## 下载和安装

### 方法一：使用提供的脚本自动下载

#### Windows
```batch
cd scripts
download_wasi_sdk.bat
```

#### Linux/macOS
```bash
cd scripts
chmod +x download_wasi_sdk.sh
./download_wasi_sdk.sh
```

脚本会自动下载最新版本的 WASI-SDK 到 `third_party/wasi` 目录。

### 方法二：手动下载

1. 访问 [WASI-SDK Releases](https://github.com/WebAssembly/wasi-sdk/releases)
2. 下载适合你平台的版本：
   - Windows: `wasi-sdk-<version>-mingw.tar.gz`
   - Linux: `wasi-sdk-<version>-linux.tar.gz`
   - macOS: `wasi-sdk-<version>-macos.tar.gz`
3. 解压到 `third_party/wasi` 目录

### 验证安装

```bash
# Windows
third_party\wasi\bin\clang.exe --version

# Linux/macOS
third_party/wasi/bin/clang --version
```

应该看到类似输出：
```
clang version X.X.X
Target: wasm32-unknown-wasi
```

## 编译 C/C++ 到 .wasm

### 基本用法

#### 编译单个文件

```bash
# Windows
third_party\wasi\bin\clang.exe -o output.wasm input.c

# Linux/macOS
third_party/wasi/bin/clang -o output.wasm input.c
```

#### 编译 C++ 文件

```bash
# Windows
third_party\wasi\bin\clang++.exe -o output.wasm input.cpp

# Linux/macOS
third_party/wasi/bin/clang++ -o output.wasm input.cpp
```

### 常用编译选项

#### 优化级别
- `-O0`: 无优化（默认，调试用）
- `-O1`: 基本优化
- `-O2`: 推荐的优化级别
- `-O3`: 激进优化
- `-Os`: 优化大小
- `-Oz`: 更激进的大小优化

#### 链接选项
- `-Wl,--no-entry`: 不需要 main 函数（用于库）
- `-Wl,--export-all`: 导出所有函数
- `-Wl,--export=func_name`: 导出指定函数
- `-Wl,--allow-undefined`: 允许未定义的符号

#### 其他选项
- `-nostdlib`: 不链接标准库
- `-fno-exceptions`: 禁用 C++ 异常
- `-std=c11` / `-std=c++17`: 指定语言标准

### 示例：编译数学库

```c
// math_lib.c
#include <math.h>

__attribute__((used))
double calculate_circle_area(double radius) {
    return 3.14159265359 * radius * radius;
}

__attribute__((used))
int add(int a, int b) {
    return a + b;
}
```

编译命令：
```bash
# Windows
third_party\wasi\bin\clang.exe ^
    -O2 ^
    -o math_lib.wasm ^
    -Wl,--export=calculate_circle_area ^
    -Wl,--export=add ^
    math_lib.c

# Linux/macOS
third_party/wasi/bin/clang \
    -O2 \
    -o math_lib.wasm \
    -Wl,--export=calculate_circle_area \
    -Wl,--export=add \
    math_lib.c
```

### 使用自动化脚本

我们提供了便捷的编译脚本：

```bash
# Windows
scripts\compile_to_wasm.bat input.c output.wasm

# Linux/macOS
scripts/compile_to_wasm.sh input.c output.wasm
```

## 编译 .wasm 到 .aot

AOT (Ahead-Of-Time) 编译可以提高 WebAssembly 的执行性能。WAMR 提供了 `wamrc` 工具进行 AOT 编译。

### 构建 wamrc 工具

首先需要构建 WAMR 的 AOT 编译器：

```bash
# 初始化子模块
git submodule update --init --recursive

# 构建 wamrc
cd third_party/wasm-micro-runtime/wamr-compiler
mkdir build && cd build

# 配置并构建
cmake ..
cmake --build .

# wamrc 可执行文件位于 build 目录
```

### 使用 wamrc 编译 AOT

#### 基本用法

```bash
# Windows
wamrc.exe -o output.aot input.wasm

# Linux/macOS
wamrc -o output.aot input.wasm
```

#### 优化选项

```bash
# 指定优化级别（0-3）
wamrc --opt-level=3 -o output.aot input.wasm

# 指定目标架构
wamrc --target=x86_64 -o output.aot input.wasm

# 可用架构：
# - x86_64
# - i386
# - aarch64
# - arm
# - thumb
# - mips
# - xtensa
# - riscv64
# - riscv32

# 启用 SIMD
wamrc --enable-simd -o output.aot input.wasm

# 组合选项
wamrc \
    --opt-level=3 \
    --target=x86_64 \
    --enable-simd \
    -o output.aot \
    input.wasm
```

### 使用自动化脚本

```bash
# Windows
scripts\compile_to_aot.bat input.wasm output.aot

# Linux/macOS
scripts/compile_to_aot.sh input.wasm output.aot
```

### AOT 性能对比

AOT 编译后的模块相比解释执行有以下优势：
- **启动速度**: AOT 模块加载更快，无需编译
- **执行速度**: 接近原生代码性能
- **内存占用**: 可以减少运行时内存开销

权衡：
- **文件大小**: AOT 文件通常比 .wasm 大
- **平台相关**: AOT 文件与目标架构绑定

## 自动化脚本

项目提供了以下脚本简化工作流程：

### 下载脚本
- `scripts/download_wasi_sdk.bat` (Windows)
- `scripts/download_wasi_sdk.sh` (Linux/macOS)

### 编译脚本
- `scripts/compile_to_wasm.bat` (Windows)
- `scripts/compile_to_wasm.sh` (Linux/macOS)
- `scripts/compile_to_aot.bat` (Windows)
- `scripts/compile_to_aot.sh` (Linux/macOS)

### 脚本使用方法

```bash
# 1. 下载 WASI-SDK
./scripts/download_wasi_sdk.sh

# 2. 编译 C 源码到 .wasm
./scripts/compile_to_wasm.sh examples/test.c build/test.wasm

# 3. 编译 .wasm 到 .aot
./scripts/compile_to_aot.sh build/test.wasm build/test.aot
```

## 示例

### 示例 1: 简单的加法函数

```c
// add.c
__attribute__((used))
__attribute__((export_name("add")))
int add(int a, int b) {
    return a + b;
}
```

编译：
```bash
third_party/wasi/bin/clang -O2 -o add.wasm add.c
```

在 CMScript 中使用：
```cpp
#include <hgl/wasm/WasmVM.h>

hgl::wasm::InitializeVM(hgl::wasm::VMType::WAMR);
auto module = hgl::wasm::CreateWasmModule(hgl::wasm::VMType::WAMR);
module->LoadFromFile("add.wasm");

auto context = hgl::wasm::CreateWasmContext(hgl::wasm::VMType::WAMR);
context->Instantiate(module);

hgl::wasm::Value args[2] = {
    hgl::wasm::Value::MakeI32(10),
    hgl::wasm::Value::MakeI32(32)
};
hgl::wasm::Value result[1];
context->CallFunction("add", args, 2, result, 1);

printf("Result: %d\n", result[0].data.i32);  // 输出: Result: 42
```

### 示例 2: 字符串处理

```c
// string_ops.c
#include <string.h>
#include <stdlib.h>

// 导出内存供 host 访问
__attribute__((export_name("memory")))
unsigned char __linear_memory[65536];

__attribute__((used))
__attribute__((export_name("string_length")))
int string_length(const char* str) {
    return strlen(str);
}

__attribute__((used))
__attribute__((export_name("reverse_string")))
void reverse_string(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}
```

### 示例 3: 数学计算

```c
// math_ops.c
#include <math.h>

__attribute__((used))
__attribute__((export_name("calculate_hypotenuse")))
double calculate_hypotenuse(double a, double b) {
    return sqrt(a * a + b * b);
}

__attribute__((used))
__attribute__((export_name("factorial")))
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
```

## 常见问题

### Q1: 编译时提示找不到 wasi-sysroot

**A**: 确保使用完整的 WASI-SDK，而不仅是 clang 编译器。WASI-SDK 包含完整的 sysroot。

### Q2: 如何减小 .wasm 文件大小？

**A**: 使用以下技巧：
1. 使用 `-Oz` 优化
2. 使用 `-Wl,--strip-all` 移除调试信息
3. 只导出需要的函数
4. 使用 `wasm-opt` 工具进一步优化

```bash
wasm-opt -Oz input.wasm -o optimized.wasm
```

### Q3: AOT 编译失败

**A**: 检查：
1. wamrc 是否正确构建
2. 目标架构是否匹配
3. .wasm 文件是否有效

### Q4: 如何在 .wasm 中使用 C++ 标准库？

**A**: WASI-SDK 支持 C++ 标准库，但某些功能受限：
```bash
clang++ -std=c++17 -o output.wasm input.cpp
```

注意：异常和 RTTI 可能需要额外配置。

### Q5: 如何调试 WebAssembly 代码？

**A**: 
1. 编译时添加调试信息：`-g`
2. 使用 wasmtime 或浏览器开发工具
3. 使用 `wasm-objdump` 查看模块内容：
```bash
wasm-objdump -x module.wasm
```

## 参考资源

- [WASI-SDK GitHub](https://github.com/WebAssembly/wasi-sdk)
- [WAMR Documentation](https://github.com/bytecodealliance/wasm-micro-runtime)
- [WebAssembly Specification](https://webassembly.github.io/spec/)
- [WASI Specification](https://github.com/WebAssembly/WASI)

## 下一步

- 查看 [examples/wasm](../examples/) 目录中的完整示例
- 阅读 [WASM VM 文档](../src/WasmVM/README.md)
- 尝试构建自己的 WebAssembly 模块
