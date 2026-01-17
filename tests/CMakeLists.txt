cmake_minimum_required(VERSION 3.25)

enable_testing()

project(TJSCompilerTests CXX)

include(CTest)
include(Catch)

set(TEST_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(TEST_FILES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test_files)
configure_file(test_config.h.in test_config.h)

set(SOURCES
        function_test.cpp
        scope_test.cpp
        oop_test.cpp
        property_test.cpp
)

string(REPLACE ".cpp" "" BASENAMES_SOURCES "${SOURCES}")
set(TARGETS ${BASENAMES_SOURCES})

foreach(name ${TARGETS})
    add_executable(${name} ${name}.cpp main.cpp)
endforeach()

set(ALL_TARGETS ${TARGETS})

foreach(name ${ALL_TARGETS})
    target_link_libraries(${name}
        PRIVATE
            Catch2::Catch2
            TJSCompiler
    )
    target_include_directories(${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    catch_discover_tests(${name})
endforeach()