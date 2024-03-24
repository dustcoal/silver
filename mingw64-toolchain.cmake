#set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")


set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_INSTALL_LIBDIR "${CMAKE_CURRENT_LIST_DIR}/../cmake")

# mingw64-toolchain.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Adjust the paths below to match the location of your MinGW-w64 installation
set(CMAKE_C_COMPILER "${CMAKE_CURRENT_LIST_DIR}/../mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_LIST_DIR}/../mingw64/bin/g++.exe")
set(CMAKE_RC_COMPILER "${CMAKE_CURRENT_LIST_DIR}/../mingw64/bin/windres.exe")

# Specify the root path for the MinGW-w64 installation
set(CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../mingw64")

# Add ../mingw/lib directory to the system PATH
#set(ENV{PATH} "${MINGW_ROOT_PATH}/lib;$ENV{PATH}")
#set(ENV{PATH} "${MINGW_ROOT_PATH}/bin;$ENV{PATH}")

# Specify the path to the make program
set(CMAKE_MAKE_PROGRAM "${CMAKE_CURRENT_LIST_DIR}/../mingw64/bin/mingw32-make.exe")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)