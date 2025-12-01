cmake_minimum_required(VERSION 3.14)

include(FetchContent)

# ensure LSPs know about what this file does
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable compile_commands.json generation")

project(main
  LANGUAGES CXX C
)

add_executable(main
  src/main.cpp
  src/app.cpp
  src/camera.cpp
  src/shader.cpp
)

# fetch GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad STATIC REPRODUCIBLE LOADER API gl:core=4.5)
target_link_libraries(main PRIVATE glad)

# fetch GLM
FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	      1.0.2
)
FetchContent_MakeAvailable(glm)

# fetch SDL2 and ensure all necessary features (and some others) are enabled
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
    GIT_TAG        release-2.30.0
)
FetchContent_MakeAvailable(SDL2)

# OpenGL linked through GLAD
target_link_libraries(main PRIVATE
  glad
  glm::glm
  SDL2::SDL2
  SDL2::SDL2main
)

# pass PROJECT_ROOT to cpp files
# so that shaders can be found using an absolute path
# instead of being relative to the executable
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
target_compile_definitions(main PRIVATE
  PROJECT_ROOT=\"${PROJECT_ROOT}\"
)

