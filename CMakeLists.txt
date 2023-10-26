cmake_minimum_required(VERSION 3.26)
project(Penguin)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(Penguin
        LexicalAnalyzer.cpp
        Main.h
        Math.h
        Penguin.cpp
        LexicalAnalyzer.h
        SyntaxAnalyzer.cpp)
