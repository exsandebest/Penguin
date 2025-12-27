cmake_minimum_required(VERSION 3.26)
project(Penguin)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

set(PENGUIN_SOURCES
        src/LexicalAnalyzer.cpp
        src/Main.h
        src/Math.h
        src/LexicalAnalyzer.h
        src/SyntaxAnalyzer.cpp
        src/SyntaxAnalyzer.h
        src/Executor.cpp
        src/Executor.h)

add_library(penguin_lib ${PENGUIN_SOURCES})
target_include_directories(penguin_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(Penguin src/Penguin.cpp)
target_link_libraries(Penguin PRIVATE penguin_lib)

enable_testing()
add_executable(PenguinTests tests/test_main.cpp)
target_link_libraries(PenguinTests PRIVATE penguin_lib)
add_test(NAME penguin_unit_tests COMMAND PenguinTests)
