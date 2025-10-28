# Base config
if(USE_EMSCRIPTEN)
        set(CMAKE_TOOLCHAIN_FILE "${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
endif()

cmake_minimum_required(VERSION 3.10)
project(luabsge CXX)

# Extensions
include(FetchContent)
include(ExternalProject)

# GLFW
option(USE_EMSCRIPTEN "Build for Emscripten/Web" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)

# Configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# WASM Build option
if(USE_EMSCRIPTEN)

        file(GLOB SOURCES src/*.cpp)

        # optimize build
        if(NOT CMAKE_BUILD_TYPE)
                set(CMAKE_BUILD_TYPE Release)
        endif()

        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/public")
        add_executable(${PROJECT_NAME} ${SOURCES})

        message(STATUS "Setting compilation target to WASM")
        set(CMAKE_EXECUTABLE_SUFFIX ".wasm.js")

        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ELIMINATE_DUPLICATE_FUNCTIONS=1 --closure 1 -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY=1 -O3 -s WASM=1 -s EXPORTED_FUNCTIONS='[_main,_js_set_int,_js_get_int]' -s EXPORTED_RUNTIME_METHODS='[ccall,cwrap]' --preload-file ${CMAKE_SOURCE_DIR}/projects/${PROJ}@/ -s USE_WEBGL2=1 -s FULL_ES3=1")

        FetchContent_Declare(
                glfw3
                GIT_REPOSITORY https://github.com/pongasoft/emscripten-glfw
        )

        FetchContent_MakeAvailable(glfw3)
        target_include_directories(${PROJECT_NAME} PRIVATE ${glfw3_SOURCE_DIR}/include)
        target_include_directories(${PROJECT_NAME} PRIVATE ${glfw3_SOURCE_DIR}/src)
        target_link_libraries(${PROJECT_NAME} PRIVATE glfw3)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_EMSCRIPTEN=1)

        # Build faster pls
        target_link_options(${PROJECT_NAME} PRIVATE
                "-Wl,--threads=20"
                "-Wl,--lto-partitions=20"
                "-Wl,--lto-O0"
                "-Wl,--lto-CGO0")

else()

        # Engine executable
        add_executable(${PROJECT_NAME})
        target_sources(${PROJECT_NAME} PRIVATE src/main.cpp)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

        # GLFW build
        FetchContent_Declare(
                glfw
                GIT_REPOSITORY https://github.com/glfw/glfw
                GIT_TAG 3.4
        )

        FetchContent_MakeAvailable(glfw)
        target_include_directories(${PROJECT_NAME} PRIVATE ${glfw_SOURCE_DIR}/include)
        target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
endif()

# Project files
file(GLOB_RECURSE SOURCES "src/*.cpp")
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# Native Lua
FetchContent_Declare(
    lua_static
    GIT_REPOSITORY https://github.com/walterschell/Lua
)

FetchContent_MakeAvailable(lua_static)
target_include_directories(${PROJECT_NAME} PRIVATE ${lua_static_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE lua_static)

# FreeType from source
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/aseprite/freetype2.git
    GIT_TAG master
)

FetchContent_MakeAvailable(freetype)

target_include_directories(${PROJECT_NAME} PRIVATE ${freetype_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE freetype)

# Assimp
FetchContent_Declare(
    assimp
    URL https://github.com/assimp/assimp/archive/refs/tags/v6.0.2.tar.gz
)

FetchContent_MakeAvailable(assimp)
target_include_directories(${PROJECT_NAME} PRIVATE ${assimp_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)

# sol2
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.5.0
)

FetchContent_MakeAvailable(sol2)
add_definitions(-DSOL_ALL_SAFETIES_ON=1)

target_include_directories(${PROJECT_NAME} PRIVATE ${sol2_SOURCE_DIR}/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SOL_ALL_SAFETIES_ON=1
    SOL_PRINT_ERRORS=1
)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
)

FetchContent_MakeAvailable(glm)

# GLM is header-only, just need to include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${glm_SOURCE_DIR})

# imgui
target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1)

# Jolt physics
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.3.0"
        SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)

target_include_directories(${PROJECT_NAME} PRIVATE ${JoltPhysics_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} PRIVATE Jolt)

# Entt
FetchContent_Declare(
        EnTT
        GIT_REPOSITORY "https://github.com/skypjack/entt"
        GIT_TAG "v3.15.0"
)
FetchContent_MakeAvailable(EnTT)

target_include_directories(${PROJECT_NAME} PRIVATE ${EnTT_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT)

