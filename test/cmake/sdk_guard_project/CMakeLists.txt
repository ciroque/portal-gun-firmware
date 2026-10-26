cmake_minimum_required(VERSION 3.22)
project(sdk_guard_fixture LANGUAGES NONE)
include("${PORTAL_ROOT}/cmake/CheckEspIdfVersion.cmake")
portal_enforce_esp_idf_version("${PORTAL_PIN}" "${PORTAL_SDK}")
