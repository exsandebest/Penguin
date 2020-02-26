@echo off
echo TEST: 1
Penguin.exe examples/1.peng
echo Expected status: ERROR
echo.
echo TEST: 2
Penguin.exe examples/2.peng
echo Expected status: OK
echo.
echo TEST: 3
Penguin.exe examples/3.peng
echo Expected status: ERROR
echo.
echo TEST: 4
Penguin.exe examples/4.peng
echo Expected status: OK
echo.
echo TEST: 5
Penguin.exe examples/5.peng
echo Expected status: ERROR
echo.
echo TEST: 6
Penguin.exe examples/6.peng
echo Expected status: ERROR
echo.
echo TEST: 7
Penguin.exe examples/7.peng
echo Expected status: OK
