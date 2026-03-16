# Build Scripts

本目录包含用于 WebAssembly 开发的自动化脚本。

## 脚本列表

### WASI-SDK 下载脚本

- **download_wasi_sdk.bat** (Windows)
- **download_wasi_sdk.sh** (Linux/macOS)

自动下载并安装 WASI-SDK 到 `third_party/wasi` 目录。

使用方法：
```bash
# Windows
download_wasi_sdk.bat

# Linux/macOS
chmod +x download_wasi_sdk.sh
./download_wasi_sdk.sh
```

### 编译到 .wasm 脚本

- **compile_to_wasm.bat** (Windows)
- **compile_to_wasm.sh** (Linux/macOS)

将 C/C++ 源代码编译为 WebAssembly (.wasm) 格式。

使用方法：
```bash
# Windows
compile_to_wasm.bat input.c output.wasm [flags]

# Linux/macOS
./compile_to_wasm.sh input.c output.wasm [flags]
```

示例：
```bash
# 基本编译
./compile_to_wasm.sh examples/wasm/add.c build/add.wasm

# 带优化
./compile_to_wasm.sh examples/wasm/add.c build/add.wasm -O2

# 导出函数
./compile_to_wasm.sh lib.c lib.wasm "-Wl,--export=my_func"
```

### 编译到 .aot 脚本

- **compile_to_aot.bat** (Windows)
- **compile_to_aot.sh** (Linux/macOS)

将 .wasm 文件编译为 AOT (.aot) 格式以提高性能。

使用方法：
```bash
# Windows
compile_to_aot.bat input.wasm output.aot [flags]

# Linux/macOS
./compile_to_aot.sh input.wasm output.aot [flags]
```

示例：
```bash
# 基本 AOT 编译
./compile_to_aot.sh build/add.wasm build/add.aot

# 带优化
./compile_to_aot.sh build/add.wasm build/add.aot --opt-level=3

# 指定架构
./compile_to_aot.sh build/add.wasm build/add.aot --target=x86_64
```

## 完整工作流

```bash
# 1. 下载 WASI-SDK（首次使用）
./download_wasi_sdk.sh

# 2. 编译 C 代码到 .wasm
./compile_to_wasm.sh examples/wasm/add.c build/add.wasm -O2

# 3. （可选）编译到 .aot 以提高性能
./compile_to_aot.sh build/add.wasm build/add.aot --opt-level=3

# 4. 在 CMScript 中使用编译好的模块
```

## 脚本特性

所有脚本都包含：

- ✅ 错误检查和验证
- ✅ 详细的帮助信息
- ✅ 跨平台支持
- ✅ 自动创建输出目录
- ✅ 文件大小报告
- ✅ 安装验证

## 前提条件

### 编译到 .wasm
- WASI-SDK（使用 download_wasi_sdk 脚本安装）

### 编译到 .aot
- WAMR 子模块（已初始化）
- 已构建的 wamrc 编译器

构建 wamrc：
```bash
git submodule update --init --recursive
cd third_party/wasm-micro-runtime/wamr-compiler
mkdir build && cd build
cmake ..
cmake --build .
```

## 更多信息

- [WASI-SDK 使用指南](../docs/WASI_SDK_GUIDE.md) - 完整文档
- [快速参考](../docs/WASI_SDK_QUICK_REFERENCE.md) - 常用命令
- [示例代码](../examples/wasm/) - C/C++ 示例

## 故障排除

### 问题：脚本没有执行权限（Linux/macOS）
```bash
chmod +x *.sh
```

### 问题：找不到 WASI-SDK
运行下载脚本：
```bash
./download_wasi_sdk.sh
```

### 问题：找不到 wamrc
需要先构建 WAMR 编译器（见上述说明）

### 问题：编译失败
1. 检查输入文件路径
2. 验证 WASI-SDK 安装
3. 查看错误信息
4. 参考完整使用指南
