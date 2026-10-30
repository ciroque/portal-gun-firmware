# Build-time companion to CheckEspIdfVersion.cmake's configure-time validation.
cmake_minimum_required(VERSION 3.22)
include("${CMAKE_CURRENT_LIST_DIR}/CheckEspIdfVersion.cmake")
portal_verify_esp_idf("${PORTAL_PIN}" "${PORTAL_SDK}" "${PORTAL_GIT}")
