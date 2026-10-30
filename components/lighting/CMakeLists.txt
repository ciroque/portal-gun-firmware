# Pure frame rendering is paired with the RMT ring and GPIO charge-light adapter.
idf_component_register(SRCS "lighting.cpp" "ring.cpp" INCLUDE_DIRS "include"
    REQUIRES config platform esp_driver_gpio esp_driver_rmt)
# IDF appends its own language flag after CMake's standard setting.
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
