idf_component_register(SRCS "controller.cpp" INCLUDE_DIRS "include" REQUIRES config inputs lighting)
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
