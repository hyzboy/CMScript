cmake_minimum_required(VERSION 3.28)

set(EXAMPLE_NAME CMScriptHelloDevil)

add_executable(${EXAMPLE_NAME}
    hello_devilvm.cpp
)

target_link_libraries(${EXAMPLE_NAME}
    PRIVATE
        CMScript
        CMCore
        CMPlatform
        CMUtil
)

target_include_directories(${EXAMPLE_NAME}
    PRIVATE
        ${CMSCRIPT_ROOT_SOURCE_PATH}/DevilVM
)

set_property(TARGET ${EXAMPLE_NAME} PROPERTY FOLDER "Examples/CMScript")
