idf_component_register(SRCS "settings.cpp" "store.cpp" INCLUDE_DIRS "include" REQUIRES nvs_flash)
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
