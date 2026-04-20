# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(MonoDeck_v2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(MonoDeck_v2 
    MonoDeck_v2.c 
    platform/platform_init.c
    platform/platform_time.c
    platform/platform_storage.c
    core/app_state.c
    core/engine.c
    core/input.c
    core/boot.c
    drivers/buttons/buttons.c
    drivers/sh110x/sh110x.c
    menu/menu.c
    games/snake/snake.c
    games/breakout/breakout.c
    games/pong/pong.c
    games/tetricore/tetricore.c
    games/flappy/flappy.c
    assets/monodeck_logo.c
    app_registry/app_registry.c
)

pico_set_program_name(MonoDeck_v2 "MonoDeck_v2")
pico_set_program_version(MonoDeck_v2 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_usb(MonoDeck_v2 1)
pico_enable_stdio_uart(MonoDeck_v2 0)

# Add the standard include files to the build
target_include_directories(MonoDeck_v2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/platform
        ${CMAKE_CURRENT_LIST_DIR}/core
        ${CMAKE_CURRENT_LIST_DIR}/drivers
        ${CMAKE_CURRENT_LIST_DIR}/drivers/sh110x
        ${CMAKE_CURRENT_LIST_DIR}/drivers/buttons
        ${CMAKE_CURRENT_LIST_DIR}/games
        ${CMAKE_CURRENT_LIST_DIR}/games/snake
        ${CMAKE_CURRENT_LIST_DIR}/games/breakout
        ${CMAKE_CURRENT_LIST_DIR}/games/pong
        ${CMAKE_CURRENT_LIST_DIR}/games/tetricore
        ${CMAKE_CURRENT_LIST_DIR}/menu
        ${CMAKE_CURRENT_LIST_DIR}/assets
        ${CMAKE_CURRENT_LIST_DIR}/app_registry
)

# Add any user requested libraries
target_link_libraries(MonoDeck_v2 
        pico_stdlib
        # hardware_spi
        hardware_i2c
        hardware_timer
        # hardware_watchdog
        # hardware_clocks
        hardware_adc
        )

pico_add_extra_outputs(MonoDeck_v2)
