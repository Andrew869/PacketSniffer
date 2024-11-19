cmake_minimum_required(VERSION 3.0)
project(DisplayProgram)

# Añadir el ejecutable
add_executable(display display.cpp)

# Encontrar la biblioteca ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(display ${CURSES_LIBRARIES})
