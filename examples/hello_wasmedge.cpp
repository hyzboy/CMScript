#include <iostream>
#include <hgl/wasm/WasmVM.h>

// Simple WASM module that adds two numbers
// WAT (WebAssembly Text format):
// (module
//   (func (export "add") (param i32 i32) (result i32)
//     local.get 0
//     local.get 1
//     i32.add)
// )

// Binary representation (compiled from WAT)
const unsigned char wasm_add_module[] = {
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x07, 0x01, 0x60, 0x02, 0x7f, 0x7f, 0x01,
    0x7f, 0x03, 0x02, 0x01, 0x00, 0x07, 0x07, 0x01,
    0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09,
    0x01, 0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a,
    0x0b
};

int main()
{
    std::cout << "=== WasmEdge Example ===" << std::endl;

    // Initialize WasmEdge VM
    if (!hgl::wasm::InitializeVM(hgl::wasm::VMType::WasmEdge))
    {
        std::cerr << "Failed to initialize WasmEdge" << std::endl;
        return 1;
    }

    // Create module
    auto module = hgl::wasm::CreateWasmModule(hgl::wasm::VMType::WasmEdge);
    if (!module)
    {
        std::cerr << "Failed to create WASM module" << std::endl;
        hgl::wasm::CleanupVM(hgl::wasm::VMType::WasmEdge);
        return 1;
    }

    // Load WASM module from memory
    if (!module->LoadFromMemory(wasm_add_module, sizeof(wasm_add_module)))
    {
        std::cerr << "Failed to load WASM module: " << module->GetErrorMessage() << std::endl;
        hgl::wasm::CleanupVM(hgl::wasm::VMType::WasmEdge);
        return 1;
    }

    std::cout << "WASM module loaded successfully" << std::endl;

    // Create execution context
    auto context = hgl::wasm::CreateWasmContext(hgl::wasm::VMType::WasmEdge);
    if (!context)
    {
        std::cerr << "Failed to create WASM context" << std::endl;
        hgl::wasm::CleanupVM(hgl::wasm::VMType::WasmEdge);
        return 1;
    }

    // Instantiate module
    if (!context->Instantiate(module))
    {
        std::cerr << "Failed to instantiate module: " << context->GetErrorMessage() << std::endl;
        hgl::wasm::CleanupVM(hgl::wasm::VMType::WasmEdge);
        return 1;
    }

    std::cout << "WASM module instantiated successfully" << std::endl;

    // Call the add function
    hgl::wasm::Value args[2];
    args[0] = hgl::wasm::Value::MakeI32(10);
    args[1] = hgl::wasm::Value::MakeI32(32);

    hgl::wasm::Value result[1];
    if (!context->CallFunction("add", args, 2, result, 1))
    {
        std::cerr << "Failed to call function: " << context->GetErrorMessage() << std::endl;
        hgl::wasm::CleanupVM(hgl::wasm::VMType::WasmEdge);
        return 1;
    }

    std::cout << "Result: 10 + 32 = " << result[0].data.i32 << std::endl;

    // Cleanup
    hgl::wasm::CleanupVM(hgl::wasm::VMType::WasmEdge);

    std::cout << "WasmEdge example completed successfully!" << std::endl;
    return 0;
}
