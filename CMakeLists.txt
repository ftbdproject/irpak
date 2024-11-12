# CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.15)
project(IRPack VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# VCPKG integration
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

# Dependencies
find_package(JUCE CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

# Project structure
set(SOURCE_FILES
    src/main.cpp
    src/AudioPackager.cpp
    src/IRProcessor.cpp
    src/Encryption.cpp
)

set(HEADER_FILES
    include/AudioPackager.hpp
    include/IRProcessor.hpp
    include/Encryption.hpp
)

# Create executable
add_executable(irpack ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(irpack
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(irpack
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_core
        ZLIB::ZLIB
        OpenSSL::SSL
        OpenSSL::Crypto
)

# Windows specific settings
if(WIN32)
    target_compile_definitions(irpack
        PRIVATE
            _WIN32_WINNT=0x0601
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif()

# Installation
install(TARGETS irpack
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)