idf_component_register(SRCS "wav.cpp" "audio.cpp" INCLUDE_DIRS "include"
    REQUIRES controller platform esp_driver_i2s esp_timer
    EMBED_FILES "../../assets/audio/portalgun_powerup1.wav"
        "../../assets/audio/portalgun_shoot_blue1.wav"
        "../../assets/audio/portalgun_shoot_red1.wav"
        "../../assets/audio/portal_invalid_surface3.wav")
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
