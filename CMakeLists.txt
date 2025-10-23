cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 17) # AUTOSAR C++14
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------
# Pico SDK
# -----------------------------------------------------
# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# initialize the Pico SDK based on PICO_SDK_PATH
include(pico_sdk_import.cmake)

project(LightDancer CXX C ASM) # Pico SDK contains C and ASM files to compile

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# -----------------------------------------------------



# Application
add_executable(LightDancer
    src/main.cpp
    src/ws2811.cpp
)
# assemble PIO program into a C header (must be done after sources specified (e.g. in either
# add_executable or target_sources)
pico_generate_pio_header(LightDancer ${PROJECT_SOURCE_DIR}/src/ws2811.pio)

# Linker option to generate a map file for inspection (does not work cross-compiling on arm
# "ld: unrecognised emulation mode: ap Supported emulations: armelf")
#target_link_options(LightDancer PUBLIC
#   "-Wl,-map,/Users/nozza/lightdancer/map.map"
#)
target_link_libraries(LightDancer pico_stdlib hardware_pio)

# Dependancies
include(FetchContent)

# Dependancy: Embedded Template Library
FetchContent_Declare(
  etl
  GIT_REPOSITORY https://github.com/ETLCPP/etl
  GIT_TAG        20.43.4
)
FetchContent_MakeAvailable(etl)

include_directories(etl INTERFACE "${FETCHCONTENT_BASE_DIR}/etl-src/include") # header only library
# include_directories(LightDancer PRIVATE ${PROJECT_SOURCE_DIR}/src/kissfft)
# Testing
#enable_testing()
#add_subdirectory(test)





pico_add_extra_outputs(LightDancer)