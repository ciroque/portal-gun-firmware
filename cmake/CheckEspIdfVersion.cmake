# Kept independent of ESP-IDF so the configuration contract is host-testable.
# Validate that the pin, active IDF_PATH version, and Git release tag agree.
function(portal_check_esp_idf_version pin_file active_version reported_version)
    if(NOT EXISTS "${pin_file}")
        message(FATAL_ERROR "Missing ESP-IDF version pin: ${pin_file}")
    endif()
    file(READ "${pin_file}" required_version)
    string(STRIP "${required_version}" required_version)
    if(NOT required_version MATCHES "^v[0-9]+\\.[0-9]+\\.[0-9]+$")
        message(FATAL_ERROR "Invalid ESP-IDF version pin: expected vMAJOR.MINOR.PATCH in ${pin_file}")
    endif()
    if(NOT "v${active_version}" STREQUAL required_version OR
       NOT reported_version STREQUAL required_version)
        message(FATAL_ERROR
            "This project requires ESP-IDF ${required_version}; active SDK is "
            "${reported_version} (${active_version}). Activate the clean pinned "
            "checkout with its export.sh; see docs/development-setup.md.")
    endif()
endfunction()

# Verify both SDK provenance and the exact version from a checkout directory.
function(portal_verify_esp_idf pin_file sdk_path git_executable)
    execute_process(
        COMMAND "${git_executable}" -C "${sdk_path}" describe --tags --dirty --always
        OUTPUT_VARIABLE description OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE git_error ERROR_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE git_result)
    if(NOT git_result EQUAL 0)
        message(FATAL_ERROR
            "Cannot identify ESP-IDF checkout at ${sdk_path} (Git result ${git_result}).\n"
            "Git diagnostic: ${git_error}\n"
            "Resolve the reported ownership/access problem or use a Git-based SDK "
            "installation; see docs/development-setup.md.")
    endif()
    if(NOT EXISTS "${sdk_path}/tools/cmake/version.cmake")
        message(FATAL_ERROR "Missing SDK version information: ${sdk_path}/tools/cmake/version.cmake")
    endif()
    include("${sdk_path}/tools/cmake/version.cmake")
    portal_check_esp_idf_version("${pin_file}"
        "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}" "${description}")
endfunction()

# Enforce the SDK at configure time and again before every build.
function(portal_enforce_esp_idf_version pin_file sdk_path)
    find_package(Git REQUIRED)
    portal_verify_esp_idf("${pin_file}" "${sdk_path}" "${GIT_EXECUTABLE}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${pin_file}")
    # Always run: HEAD/index dependencies alone miss unstaged tracked-file edits.
    add_custom_target(portal_sdk_guard ALL
        COMMAND "${CMAKE_COMMAND}"
            "-DPORTAL_PIN=${pin_file}" "-DPORTAL_SDK=${sdk_path}"
            "-DPORTAL_GIT=${GIT_EXECUTABLE}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/VerifyEspIdf.cmake"
        COMMENT "Verifying the pinned, clean ESP-IDF checkout"
        VERBATIM)
endfunction()
