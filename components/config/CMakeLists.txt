# Settings codec plus the ESP-IDF NVS persistence adapter.
idf_component_register(SRCS "settings.cpp" "store.cpp" INCLUDE_DIRS "include" REQUIRES nvs_flash)
# Keep the portable codec on the same C++17 language level as the firmware.
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
