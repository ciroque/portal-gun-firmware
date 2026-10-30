# Physical button sampling is the only GPIO-backed input component.
idf_component_register(SRCS "gpio_inputs.cpp" INCLUDE_DIRS "include" REQUIRES platform esp_driver_gpio)
# Match the language level used by the portable debounce and controller code.
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
