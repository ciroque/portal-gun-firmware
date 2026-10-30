# Compose the prop runtime from hardware adapters and the portable controller.
idf_component_register(SRCS "main.cpp" REQUIRES controller platform config inputs lighting
    audio log esp_timer esp_hw_support)
# IDF appends its own language flag after CMake's CXX_STANDARD flag. Override
# it in the component's options so the effective (last) flag is C++17.
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
