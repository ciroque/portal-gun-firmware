# Embed only the four selected, converted playback WAVs; source recordings stay in wav/.
idf_component_register(SRCS "wav.cpp" "audio.cpp" INCLUDE_DIRS "include"
    REQUIRES controller platform esp_driver_i2s esp_timer
    EMBED_FILES "../../assets/audio/portalgun_powerup1.wav"
        "../../assets/audio/portalgun_shoot_blue1.wav"
        "../../assets/audio/portalgun_shoot_red1.wav"
        "../../assets/audio/portal_invalid_surface3.wav")
# IDF appends its own language flag, so keep this component's effective standard at C++17.
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>")
