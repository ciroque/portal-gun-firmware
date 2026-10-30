# Project entry point: require the pinned ESP-IDF checkout before IDF configures.
cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED ENV{IDF_PATH} OR "$ENV{IDF_PATH}" STREQUAL "")
    message(FATAL_ERROR "Activate the pinned ESP-IDF with export.sh; see docs/development-setup.md")
endif()

include(cmake/CheckEspIdfVersion.cmake)
portal_enforce_esp_idf_version("${CMAKE_CURRENT_SOURCE_DIR}/.esp-idf-version" "$ENV{IDF_PATH}")

include("$ENV{IDF_PATH}/tools/cmake/project.cmake")
if(NOT IDF_TARGET STREQUAL "esp32s3")
    message(FATAL_ERROR "This board requires IDF_TARGET=esp32s3, not ${IDF_TARGET}")
endif()

project(portal_gun)
add_dependencies(portal_gun.elf portal_sdk_guard)
