cmake_minimum_required(VERSION 3.22)
include("${PORTAL_ROOT}/cmake/CheckEspIdfVersion.cmake")
portal_enforce_esp_idf_version("${PORTAL_PIN}" "${PORTAL_SDK}")
# Match the firmware's order: validate the SDK before project initialization.
project(sdk_guard_fixture LANGUAGES NONE)
