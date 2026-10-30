# State machine depends on subsystem interfaces, not board-specific drivers.
idf_component_register(SRCS "controller.cpp" INCLUDE_DIRS "include" REQUIRES config inputs lighting)
# IDF appends its own language flag after CMake's standard setting.
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
