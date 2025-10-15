cmake_minimum_required(VERSION 3.2)

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
target_link_libraries(main PRIVATE glm::glm)

find_package(SDL2 REQUIRED)
target_link_libraries(main PRIVATE SDL2)

find_package(glew REQUIRED)
target_link_libraries(main PRIVATE GLEW::GLEW)

# pass PROJECT_ROOT to cpp files
# so that shaders can be found using an absolute path
# instead of being relative to the executable
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
target_compile_definitions(main PRIVATE
  PROJECT_ROOT=\"${PROJECT_ROOT}/../\"
)

