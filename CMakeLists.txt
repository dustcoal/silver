cmake_minimum_required(VERSION 2.8)

project(craft)

FILE(GLOB SOURCE_FILES src/*.c)

FILE(GLOB COMMON_SOURCE_FILES
        src/common/common.c
        src/common/logging.c
        )

FILE(GLOB SERVER_SOURCE_FILES
        src/server/server.c
        src/server/config_main.c
        )

add_executable(
    craft
    ${SOURCE_FILES}
    deps/glew/src/glew.c
    deps/lodepng/lodepng.c
    deps/noise/noise.c
    deps/sqlite/sqlite3.c
    deps/tinycthread/tinycthread.c
    deps/cJSON/cJSON.c
)

add_executable(
    server
        src/world.c
    ${COMMON_SOURCE_FILES}
        ${SERVER_SOURCE_FILES}
    deps/lodepng/lodepng.c # needed for server-side image validation for skin upload
    deps/noise/noise.c
    deps/sqlite/sqlite3.c
    deps/tinycthread/tinycthread.c
    deps/cJSON/cJSON.c
)

add_definitions(-std=c99 -O3)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/glfw)

include_directories(src/headers)
include_directories(deps/glew/include)
include_directories(deps/glfw/include)
include_directories(deps/lodepng)
include_directories(deps/noise)
include_directories(deps/sqlite)
include_directories(deps/tinycthread)
include_directories(deps/cJSON)

if(MINGW)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
        "C:/Program Files/CURL/lib" "C:/Program Files (x86)/CURL/lib" "deps/curl_windows/lib")
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
        "C:/Program Files/CURL/include" "C:/Program Files (x86)/CURL/include" "deps/curl_windows/include")
endif()

if(WIN32 AND NOT MINGW)
    set(CURL_INCLUDE_DIR "deps/curl_windows/include")
    set(CURL_LIBRARY "deps/curl_windows/lib/libcurl")
endif()

if(MINGW)
    set(CURL_INCLUDE_DIR "deps/curl_windows/include")
    #set(CURL_LIBRARY "deps/curl_windows/lib/libcurl.lib")
endif()

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

if(APPLE)
    target_link_libraries(craft glfw
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()

if(UNIX)
    target_link_libraries(craft dl glfw GL
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()

if(MINGW)
    target_link_libraries(craft ${CURL_LIBRARIES}
        ws2_32.lib wldap32.lib advapi32.lib kernel32.lib comdlg32.lib crypt32.lib normaliz.lib glfw opengl32
        ${GLFW_LIBRARIES})
endif()

if(WIN32)
    target_link_libraries(craft ${CURL_LIBRARIES} ws2_32.lib wldap32.lib advapi32.lib kernel32.lib comdlg32.lib crypt32.lib normaliz.lib glfw opengl32 ${GLFW_LIBRARIES})
endif()
