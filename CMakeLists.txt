cmake_minimum_required(VERSION 3.20)
project(glfwdraft CXX)

# Clangを使用するように指定
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

message(STATUS "Current Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIRS}
)

#実行ファイルの設定
add_executable(${PROJECT_NAME} main.cpp lib/Matrix)

#リンクするライブラリ
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    glfw3
    stdc++
)

set(CMAKE_VERBOSE_MAKEFILE ON)