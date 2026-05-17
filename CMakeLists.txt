cmake_minimum_required(VERSION 4.2)
project(warcaby C)

set(CMAKE_C_STANDARD 11)

add_executable(warcaby main.c
        board.h
        board.c)
