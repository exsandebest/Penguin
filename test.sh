#!/bin/bash
#Run this in terminal
#Makefile for Penguin language
echo "TEST: 1"
Penguin examples/1.peng
echo "Expected status: ERROR"
echo "TEST: 2"
Penguin examples/2.peng
echo "Expected status: OK"
echo "TEST: 3"
Penguin examples/3.peng
echo "Expected status: ERROR"
echo "TEST: 4"
Penguin examples/4.peng
echo "Expected status: OK"
echo "TEST: 5"
Penguin examples/5.peng
echo "Expected status: ERROR"
echo "TEST: 6"
Penguin.exe examples/6.peng
echo "Expected status: ERROR"
echo "TEST: 7"
Penguin.exe examples/7.peng
echo "Expected status: OK"
exit 0
