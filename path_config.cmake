macro(CMScriptSetup CMSCRIPT_ROOT_PATH)
    message("CMScript root path: " ${CMSCRIPT_ROOT_PATH})

    set(CMSCRIPT_ROOT_INCLUDE_PATH ${CMSCRIPT_ROOT_PATH}/inc)
    set(CMSCRIPT_ROOT_SOURCE_PATH ${CMSCRIPT_ROOT_PATH}/src)

    include_directories(${CMSCRIPT_ROOT_INCLUDE_PATH})

    include_directories(${CMSCRIPT_ROOT_SOURCE_PATH}/DevilVM)
endmacro()
