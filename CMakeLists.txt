cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED ENV{IDF_PATH} OR "$ENV{IDF_PATH}" STREQUAL "")
    message(FATAL_ERROR "Activate the pinned ESP-IDF with export.sh; see docs/development-setup.md")
endif()

include(cmake/CheckEspIdfVersion.cmake)
include("$ENV{IDF_PATH}/tools/cmake/version.cmake")
find_package(Git REQUIRED)
execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "$ENV{IDF_PATH}" describe --tags --dirty --always
    OUTPUT_VARIABLE portal_idf_description OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE portal_idf_describe_result)
if(NOT portal_idf_describe_result EQUAL 0)
    message(FATAL_ERROR "Cannot identify ESP-IDF checkout; use the recursive Git installation in docs/development-setup.md")
endif()
portal_check_esp_idf_version("${CMAKE_CURRENT_SOURCE_DIR}/.esp-idf-version"
    "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}"
    "${portal_idf_description}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/.esp-idf-version")

if(NOT DEFINED IDF_TARGET)
    if(DEFINED ENV{IDF_TARGET})
        set(IDF_TARGET "$ENV{IDF_TARGET}")
    else()
        set(IDF_TARGET esp32s3)
    endif()
endif()
if(NOT IDF_TARGET STREQUAL "esp32s3")
    message(FATAL_ERROR "This board requires IDF_TARGET=esp32s3, not ${IDF_TARGET}")
endif()

include("$ENV{IDF_PATH}/tools/cmake/project.cmake")
project(portal_gun)
