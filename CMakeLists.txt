cmake_minimum_required(VERSION 3.25)

project(TJSCompiler CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/${CMAKE_SYSTEM_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/${CMAKE_SYSTEM_NAME})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

set(GEN_CODE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)

if(NOT EXISTS ${GEN_CODE_BINARY_DIR})
    file(MAKE_DIRECTORY ${GEN_CODE_BINARY_DIR})
endif()

add_library(${PROJECT_NAME})

add_compile_definitions(${PROJECT_NAME} UNICODE _UNICODE

    # 抑制codecvt警告
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
)

find_program(BISON_EXECUTABLE NAMES win_bison.exe bison.exe bison)

set(BISON_FILES
        ${CMAKE_CURRENT_LIST_DIR}/src/syntax/bison/tjs.y
        ${CMAKE_CURRENT_LIST_DIR}/src/syntax/bison/tjsdate.y
        ${CMAKE_CURRENT_LIST_DIR}/src/syntax/bison/tjspp.y
)

set(PARSE_TAB_CPP_FILES)

foreach(BISON_FILE ${BISON_FILES})
    get_filename_component(FILE_NAME ${BISON_FILE} NAME_WE)

    execute_process(
            TIMEOUT 5000
            COMMAND ${BISON_EXECUTABLE} ${BISON_FILE}
            WORKING_DIRECTORY ${GEN_CODE_BINARY_DIR}
            COMMAND_ECHO STDOUT
    )

    add_custom_command(
        OUTPUT
            ${GEN_CODE_BINARY_DIR}/${FILE_NAME}.tab.cpp
            ${GEN_CODE_BINARY_DIR}/${FILE_NAME}.tab.hpp
        DEPENDS ${BISON_FILE}
        COMMAND ${BISON_EXECUTABLE} ${BISON_FILE}
        WORKING_DIRECTORY ${GEN_CODE_BINARY_DIR}
        COMMENT "Generate Bison File From ${BISON_FILE}"
    )
    list(APPEND PARSE_TAB_CPP_FILES ${GEN_CODE_BINARY_DIR}/${FILE_NAME}.tab.cpp)
endforeach()

add_custom_target(GENERATE_CPP_FILES DEPENDS ${PARSE_TAB_CPP_FILES})

add_dependencies(${PROJECT_NAME} GENERATE_CPP_FILES)

find_package(Python3 REQUIRED)
execute_process(
    TIMEOUT 3000
    COMMAND ${Python3_EXECUTABLE}
    create_world_map.py
    ${CMAKE_SOURCE_DIR}/src/syntax/lexer/tjsDateWordMap.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/py_script
    COMMAND_ECHO STDOUT
)

# src/syntax/lexer/tjsDateWordMap.cpp
# ----------------------------------- source ----------------------------------- #
target_sources(${PROJECT_NAME} PRIVATE
    ${PARSE_TAB_CPP_FILES}
#    src/main.cpp
    src/syntax/tjs.cpp
    src/syntax/tjsArray.cpp
    src/syntax/tjsBinarySerializer.cpp
    src/syntax/tjsByteCodeLoader.cpp
    src/syntax/tjsConfig.cpp
    src/syntax/tjsConstArrayData.cpp
    src/syntax/tjsDate.cpp
    src/syntax/tjsDebug.cpp
    src/syntax/tjsDictionary.cpp
    src/syntax/tjsDisassemble.cpp
    src/syntax/tjsError.cpp
    src/syntax/tjsException.cpp
    src/syntax/tjsGlobalStringMap.cpp
    src/syntax/tjsInterCodeExec.cpp
    src/syntax/tjsInterface.cpp
    src/syntax/tjsMath.cpp
    src/syntax/tjsMessage.cpp
    src/syntax/tjsMT19937ar-cok.cpp
    src/syntax/tjsNamespace.cpp
    src/syntax/tjsNative.cpp
    src/syntax/tjsObject.cpp
    src/syntax/tjsObjectExtendable.cpp
    src/syntax/tjsOctPack.cpp
    src/syntax/tjsRandomGenerator.cpp
    src/syntax/tjsRegExp.cpp
    src/syntax/tjsScriptCache.cpp
    src/syntax/tjsScriptBlock.cpp
    src/syntax/tjsString.cpp
    src/syntax/tjsUtils.cpp
    src/syntax/tjsVariant.cpp
    src/syntax/tjsVariantString.cpp

    src/utils/cp932_uni.cpp
    src/utils/uni_cp932.cpp

    src/syntax/lexer/tjsLex.cpp
    src/syntax/lexer/tjsCompileControl.cpp
    src/syntax/lexer/tjsDateParser.cpp
    src/syntax/lexer/tjsInterCodeGen.cpp
    src/syntax/library/DebugInterface.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${GEN_CODE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/syntax
    ${CMAKE_SOURCE_DIR}/src/syntax/lexer
    ${CMAKE_SOURCE_DIR}/src/syntax/AST
    ${CMAKE_SOURCE_DIR}/src/syntax/library
)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_TEST OFF)
set(INSTALL_DOCUMENTATION OFF)

# 如果使用MSVC需要指定导出符号
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(depend)

# --------------------------------- oniguruma --------------------------------- #
add_compile_definitions(ONIG_EXTERN=extern)

# ------------------------------ import library ------------------------------- #
target_link_libraries(${PROJECT_NAME} PRIVATE onig)

add_subdirectory(tests)