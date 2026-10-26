idf_component_register(SRCS "main.cpp" REQUIRES platform log)
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)
