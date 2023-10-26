cmake_minimum_required(VERSION 3.26)
project(Penguin)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(Penguin
        src/LexicalAnalyzer.cpp
        src/Main.h
        src/Math.h
        src/Penguin.cpp
        src/LexicalAnalyzer.h
        src/SyntaxAnalyzer.cpp
        src/SyntaxAnalyzer.h
        src/Executor.cpp
        src/Executor.h)
