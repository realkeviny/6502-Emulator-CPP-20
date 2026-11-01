cmake_minimum_required(VERSION 3.28)

project(
    6502-C++20
    VERSION 1.0.3
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(ENABLE_LTO "Link-Time Optimization" OFF)
set(PROCESSOR_NAME C85)
set(DISASSEMBLER_NAME Disassembler)

option(ENABLE_TESTING "Enable a Unit Test Build" ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(ENABLE_LTO)
    include(LTO)
endif()

option(USE_CPM "Use CPM?" OFF)
option(USE_CONAN "Use Conan?" ON)

if(USE_CPM)
    message(STATUS "Activate CPM")
    include(CPM)
    if(ENABLE_TESTING)
        cpmaddpackage("gh:catchorg/Catch2#v3.16.0")
        list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    endif()
elseif(USE_CONAN)
    message(STATUS "Activate Conan")
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

    if(ENABLE_TESTING)
        find_package(Catch2 REQUIRED)
        list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    endif()
else()
    message(STATUS "Activate FetchContent")
    include(FetchContent)
    if(ENABLE_TESTING)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2
            GIT_TAG v3.16.0
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(Catch2)
        list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    endif()
endif()

add_subdirectory(configured)
add_subdirectory(src)
add_subdirectory(app)

if(ENABLE_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
