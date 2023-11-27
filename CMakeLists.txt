cmake_minimum_required(VERSION 3.10)

project(craft)

# this makes the qoi implementation available
#add_definitions(-DQOI_IMPLEMENTATION)

add_definitions(-DENABLE_CJSON_UTILS=On -DENABLE_CJSON_TEST=Off)
add_definitions(-DLOG_USE_COLOR)

FILE(GLOB SOURCE_FILES src/*.c)

FILE(GLOB COMMON_SOURCE_FILES
    src/common/common.c
    src/common/logging.c
    src/common/config.c
    src/common/qoi_utils.c

    src/common/util/ltoa.c
    src/common/util/ulltoa.c
    src/common/util/pointer_util.c
    src/common/util/min_max.c
)

FILE(GLOB SERVER_SOURCE_FILES
    src/server/server.c
    src/server/config_main.c
)

FILE(GLOB CLIENT_SOURCE_FILES
    src/client/network.c
    src/client/world_interact.c
    src/client/chat.c
)

FILE(GLOB COMMON_DEPS
    deps/lodepng/lodepng.c # needed for server-side image validation for skin upload
    deps/noise/noise.c
    deps/sqlite/sqlite3.c
    deps/tinycthread/tinycthread.c
    deps/cJSON/cJSON.c
    deps/qoi/qoi.c
    deps/log.c/log.c
)

add_executable(
    craft

    ${SOURCE_FILES}
    ${CLIENT_SOURCE_FILES}
    ${COMMON_DEPS}
    deps/glew/src/glew.c
)

add_executable(
    server

    src/world.c
    ${COMMON_SOURCE_FILES}
    ${SERVER_SOURCE_FILES}
    ${COMMON_DEPS}
)

add_definitions(-std=gnu99 -O3)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/glfw)
add_subdirectory(deps/cwalk)

include_directories(src/headers)
include_directories(deps/glew/include)
include_directories(deps/glfw/include)
include_directories(deps/lodepng)
include_directories(deps/noise)
include_directories(deps/sqlite)
include_directories(deps/tinycthread)
include_directories(deps/cJSON)
include_directories(deps/cwalk/include)
include_directories(deps/qoi)
include_directories(deps/stb)
include_directories(deps/log.c)

#TODO: Link necessary libraries for target 'server' (it doesn't link to graphics at all (except image manipulation libs))
target_link_libraries(craft cwalk)
target_link_libraries(server cwalk)


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
