include(FetchContent)

# Set path to libtropic if not already defined
if(NOT DEFINED PATH_LIBTROPIC)
    set(PATH_LIBTROPIC "${CMAKE_CURRENT_SOURCE_DIR}/../libtropic/" CACHE PATH "Path to libtropic library")
endif()

# Select pairing keys written during manufacturing into your TROPIC01
set(LT_SH0_KEYS "prod0" CACHE STRING "Choose which pairing keys in slot 0 will be used in this example")
set_property(CACHE LT_SH0_KEYS PROPERTY STRINGS "eng_sample" "prod0")

# These are internal macros for selecting SH0 keys
set(LT_USE_SH0_ENG_SAMPLE 0 CACHE INTERNAL "")
set(LT_USE_SH0_PROD0      0 CACHE INTERNAL "")

# Define SH0 macros based on selected string
if(LT_SH0_KEYS STREQUAL "eng_sample")
    message(STATUS "Using Engineering sample keys")
    set(LT_USE_SH0_ENG_SAMPLE 1)
    set(LT_USE_SH0_PROD0      0)
elseif(LT_SH0_KEYS STREQUAL "prod0")
    message(STATUS "Using Production 0 keys")
    set(LT_USE_SH0_ENG_SAMPLE 0)
    set(LT_USE_SH0_PROD0      1)
else()
    get_property(lt_sh0_keys_choices CACHE LT_SH0_KEYS PROPERTY STRINGS)
    message(FATAL_ERROR "Incorrect SH0 keys specified: '${LT_SH0_KEYS}'\nAvailable SH0 keys: ${lt_sh0_keys_choices}")
endif()

# ========================================================================
# Libtropic 
# ========================================================================
add_subdirectory(${PATH_LIBTROPIC} "libtropic")

# ========================================================================
# MbedTLS v4.0.0
# ========================================================================

# Download MbedTLS
FetchContent_Declare(
    mbedtls_v4
    URL https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-4.0.0/mbedtls-4.0.0.tar.bz2
    URL_HASH SHA256=2f3a47f7b3a541ddef450e4867eeecb7ce2ef7776093f3a11d6d43ead6bf2827
)

# We configure MbedTLS using config file with following configuration:
# - config.py preset "crypto_baremetal"
# - following options enabled:
#   MBEDTLS_PLATFORM_MS_TIME_ALT
#   MBEDTLS_HAVE_TIME
#   MBEDTLS_PSA_DRIVER_GET_ENTROPY
# - following options disabled:
#   MBEDTLS_PSA_BUILTIN_GET_ENTROPY
#   MBEDTLS_TEST_HOOKS
#   MBEDTLS_PSA_CRYPTO_BUILTIN_KEYS
# We need to set both CMake variables and compile definitions for MbedTLS to pick up our config file.
set(MBEDTLS_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/mbedtls_v4/mbedtls_config.h")
set(TF_PSA_CRYPTO_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/mbedtls_v4/crypto_config.h")
add_compile_definitions(MBEDTLS_CONFIG_FILE="${MBEDTLS_CONFIG_FILE}")
add_compile_definitions(TF_PSA_CRYPTO_CONFIG_FILE="${TF_PSA_CRYPTO_CONFIG_FILE}")
set(ENABLE_TESTING OFF CACHE BOOL "Disable mbedtls_v4 test building.")
set(ENABLE_PROGRAMS OFF CACHE BOOL "Disable mbedtls_v4 examples building.")
FetchContent_MakeAvailable(mbedtls_v4)

# Apply optimization flags to MbedTLS
if(TARGET builtin)
    target_compile_options(builtin PRIVATE -O2)
endif()

foreach(target mbedcrypto mbedx509 mbedtls)
    if(TARGET ${target})
        target_compile_options(${target} PRIVATE -O2)
    endif()
endforeach()

# ========================================================================
# Link libtropic with MbedTLS and STM32 HAL
# ========================================================================
add_subdirectory("${PATH_LIBTROPIC}/cal/mbedtls_v4" "mbedtls_v4_cal")
target_sources(tropic PRIVATE ${LT_CAL_SRCS})
target_include_directories(tropic PUBLIC ${LT_CAL_INC_DIRS})
target_link_libraries(tropic PUBLIC mbedtls)

add_subdirectory("${PATH_LIBTROPIC}/hal/stm32/stm32u5xx" "stm32u5xx_hal")
target_sources(tropic PRIVATE ${LT_HAL_SRCS})
target_include_directories(tropic PUBLIC ${LT_HAL_INC_DIRS})
