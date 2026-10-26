# Kept independent of ESP-IDF so the configuration contract is host-testable.
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
