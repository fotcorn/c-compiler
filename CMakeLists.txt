cmake_minimum_required(VERSION 3.13)
project(selfhost_compiler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -pedantic -g)

add_executable(compiler
    src/main.c
)

configure_file(
    ${CMAKE_SOURCE_DIR}/test/lit.site.cfg.py.in
    ${CMAKE_BINARY_DIR}/test/lit.site.cfg.py
    @ONLY
)

add_custom_target(check
    COMMAND lit -v ${CMAKE_BINARY_DIR}/test
    DEPENDS compiler
    COMMENT "Running lit test suite"
)
