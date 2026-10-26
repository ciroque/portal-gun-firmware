idf_component_register(SRCS "gpio_inputs.cpp" INCLUDE_DIRS "include" REQUIRES platform esp_driver_gpio)
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
