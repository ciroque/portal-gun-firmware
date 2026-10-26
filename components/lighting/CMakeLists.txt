idf_component_register(SRCS "lighting.cpp" "ring.cpp" INCLUDE_DIRS "include"
    REQUIRES config platform esp_driver_gpio esp_driver_rmt)
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
