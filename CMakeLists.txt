cmake_minimum_required(VERSION 4.2)
project(CE6ML)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(
        safetyhook
        GIT_REPOSITORY "https://github.com/cursey/safetyhook.git"
        GIT_TAG "origin/main"
)
FetchContent_MakeAvailable(safetyhook)

add_library(CE6ML SHARED
        src/dllmain.cpp
        xinput.def
)

target_sources(CE6ML PUBLIC FILE_SET CXX_MODULES FILES
        src/multiplayerHooks.ixx
        src/utils.ixx
        src/hooks.ixx
        src/loader.ixx
)

target_include_directories(CE6ML PRIVATE
        src
        ${CMAKE_SOURCE_DIR}
        ${safetyhook_SOURCE_DIR}/include
)

target_link_libraries(CE6ML PRIVATE
        safetyhook
)

set_target_properties(CE6ML PROPERTIES OUTPUT_NAME "xinput1_3" SUFFIX ".dll")