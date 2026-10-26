cmake_minimum_required(VERSION 3.22)
project(portal_host_tests LANGUAGES CXX)

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(portal_tests board_pins_test.cpp)
set_target_properties(portal_tests PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_compile_options(portal_tests PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>")
target_include_directories(portal_tests PRIVATE
    ../third_party/doctest
    ../components/platform/include)
add_test(NAME board_pins COMMAND portal_tests)
add_test(NAME sdk_version_guard
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/sdk_version_test.py"
        "${CMAKE_COMMAND}" "${CMAKE_CURRENT_SOURCE_DIR}/..")
