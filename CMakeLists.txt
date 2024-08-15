cmake_minimum_required(VERSION 3.27)
project(compiler_files C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(compiler_files
        compiler.c
        compiler.h
        lexer.c
        lexer.h
        parser.c
        parser.h
        symbols.c
        symbols.h)
