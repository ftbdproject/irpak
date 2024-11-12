@echo off
setlocal

:: Check if VCPKG_ROOT is set
if not defined VCPKG_ROOT (
    echo Error: VCPKG_ROOT environment variable is not set
    exit /b 1
)

:: Create build directory
if not exist build mkdir build

:: Configure
cmake -B build -S . ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=install

:: Build
cmake --build build --config Release

:: Install
cmake --install build --config Release

endlocal