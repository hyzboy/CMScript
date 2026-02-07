cmake_minimum_required(VERSION 3.28)

macro(cm_example_project sub_folder project_name)
    cm_example_project_base(
        PROJECT_NAME ${project_name}
        FOLDER_PATH "Examples/CMScript/${sub_folder}"
        SOURCES ${ARGN}
        PRIVATE_LIBRARIES   CMScript
                            CMCore
                            CMPlatform
                            CMUtil
    )
    # Set C++20 standard for all CMScript examples
    set_property(TARGET ${project_name} PROPERTY CXX_STANDARD 20)
endmacro()

cm_example_project("" DevilVM_Hello hello_devilvm.cpp)
cm_example_project("" DevilVM_Goto goto_devilvm.cpp)