# Configuration
cmake_minimum_required(VERSION 3.28)
project(luabsge CXX)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Engine executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
file(GLOB_RECURSE SOURCES "src/*.cpp")
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

include(FetchContent)

# glfw
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.3.10
)
FetchContent_MakeAvailable(glfw)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# Lua
find_package(Lua REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})

# Freetype
find_package(Freetype REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${FREETYPE_LIBRARIES})

# Assimp
find_package(assimp REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ASSIMP_LIBRARIES})

# find_package(GL REQUIRED)
# find_package(X11 REQUIRED)
# find_package(pthread REQUIRED)
# find_package(assimp REQUIRED)
# find_package(Xrandr REQUIRED) 
# find_package(Xi REQUIRED)
