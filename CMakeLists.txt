cmake_minimum_required(VERSION 3.28)
project(sig_c_language C)

set(CMAKE_C_STANDARD 11)

add_executable(sig_c_language main.c)

configure_file(program.sigc ${CMAKE_BINARY_DIR}/program.sigc COPYONLY)