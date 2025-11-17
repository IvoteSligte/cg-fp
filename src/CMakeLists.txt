cmake_minimum_required(VERSION 3.2)

include(FetchContent)

# ensure LSPs know about what this file does
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable compile_commands.json generation")

project(main
  LANGUAGES CXX
)

add_executable(main
  main.cpp
  app.cpp
  camera.cpp
  shader.cpp
)

find_package(glm CONFIG REQUIRED)

set(SDL_SHARED_ENABLED_BY_DEFAULT ON)
set(SDL_STATIC_ENABLED_BY_DEFAULT OFF)
set(SDL_VIDEO ON)
set(SDL_VIDEO_OPENGL ON)
set(SDL_VIDEO_X11 ON)
set(SDL_X11_XCURSOR ON)
set(SDL_X11_XINERAMA ON)
set(SDL_X11_XINPUT2 ON)
set(SDL_X11_XRANDR ON)
set(SDL_X11_XSCRNSAVER ON)
set(SDL_X11_XV ON)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.0
)
FetchContent_MakeAvailable(SDL2)

find_package(glew REQUIRED)

target_link_libraries(main PRIVATE
  glm::glm
  GLEW::GLEW
  SDL2::SDL2
  SDL2::SDL2main
)

# pass PROJECT_ROOT to cpp files
# so that shaders can be found using an absolute path
# instead of being relative to the executable
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
target_compile_definitions(main PRIVATE
  PROJECT_ROOT=\"${PROJECT_ROOT}/../\"
)

