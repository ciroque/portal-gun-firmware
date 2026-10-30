# Test adapter for both the release-pin check and the full SDK-checkout guard.
cmake_minimum_required(VERSION 3.22)
include("${PORTAL_ROOT}/cmake/CheckEspIdfVersion.cmake")
if(PORTAL_CHECK_CHECKOUT)
    portal_verify_esp_idf("${PORTAL_PIN}" "${PORTAL_SDK}" "${PORTAL_GIT}")
else()
    portal_check_esp_idf_version("${PORTAL_PIN}" "${PORTAL_ACTIVE}" "${PORTAL_REPORTED}")
endif()
