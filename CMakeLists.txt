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

# WASM Build option
if(USE_EMSCRIPTEN)
        option(JS_ONLY "Compiles to native JS (No WASM)" OFF)

        file(GLOB SOURCES src/*.cpp)

        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/public")
        add_executable(${PROJECT_NAME} ${SOURCES})

        if(JS_ONLY)
                message(STATUS "Setting compilation target to native JavaScript")
                set(CMAKE_EXECUTABLE_SUFFIX ".js")
                set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s WASM=0 -s EXPORTED_FUNCTIONS='[_main]' --preload-file ${CMAKE_SOURCE_DIR}/projects/webtest@/ -s USE_WEBGL2=1 -s FULL_ES3=1")
        else(JS_ONLY)
                message(STATUS "Setting compilation target to WASM")
                set(CMAKE_EXECUTABLE_SUFFIX ".wasm.js")

                set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -s EXPORTED_FUNCTIONS='[_main]' --preload-file ${CMAKE_SOURCE_DIR}/projects/webtest@/ -s USE_WEBGL2=1 -s FULL_ES3=1")
        endif(JS_ONLY)

        FetchContent_Declare(
                glfw3
                GIT_REPOSITORY https://github.com/pongasoft/emscripten-glfw
        )

        FetchContent_MakeAvailable(glfw3)
        target_include_directories(${PROJECT_NAME} PRIVATE ${glfw3_SOURCE_DIR}/include)
        target_include_directories(${PROJECT_NAME} PRIVATE ${glfw3_SOURCE_DIR}/src)
        target_link_libraries(${PROJECT_NAME} PRIVATE glfw3)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_EMSCRIPTEN=1)
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