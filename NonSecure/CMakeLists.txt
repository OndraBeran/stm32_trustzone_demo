cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Compiler options
set(STM32_MCU_FLAGS  "-mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard " )

#Linker options
set(STM32_LINKER_SCRIPT STM32U545xx_FLASH_ns.ld)
set(STM32_LINKER_OPTION  )

# Set the project name
set(CMAKE_PROJECT_NAME stm32_trustzone_demo_NS)
project(${CMAKE_PROJECT_NAME})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Include mx-generated
include("mx-generated.cmake")

# Rename v1 output
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}_v1")
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-Wl,-Map=${CMAKE_PROJECT_NAME}_v1.map")

# v2 variant — uses _main.c (malicious) instead of main.c
set(_V2 "${CMAKE_PROJECT_NAME}_v2")
add_executable(${_V2})

set(_V2_App_Src ${MX_Application_Src})
list(FILTER _V2_App_Src EXCLUDE REGEX ".*[/]main\\.c$")
target_sources(${_V2} PRIVATE
    ${_V2_App_Src}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/malicious.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/i2c_lcd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/KeyPad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi_ex.c
)
target_link_libraries(${_V2} ${MX_LINK_LIBS})
target_link_options(${_V2} PRIVATE "-Wl,-Map=${_V2}.map")
set_target_properties(${_V2} PROPERTIES ADDITIONAL_CLEAN_FILES "${_V2}.map")


# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/i2c_lcd.c
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined libraries
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()









