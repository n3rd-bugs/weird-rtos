:: This script setup the build enviornment for ARM and AVR.
:: NOTE: Please set the required paths below before executing 
::       this script.
@echo off

:: Populate paths.
set ARM_TOOLSET=C:\Program Files (x86)\GNU Tools Arm Embedded\7 2018-q2-update\bin\
set AVR_TOOLSET=D:\Hobby\avr8-gnu-toolchain\bin\
set PYTHON_PATH=D:\Hobby\python\
set PYTHON_SCRIPTS_PATH=D:\Hobby\python\Scripts
set CMAKE_PATH="C:\Program Files\CMake\bin\"
set ECLIPSE_PATH="D:\Hobby\eclipse\"

:: Add ARM toolchain.
setx path "%AVR_TOOLSET%;%ARM_TOOLSET%;%PYTHON_PATH%;%PYTHON_SCRIPTS_PATH%;%%path%%" >nul 2>&1

:: Execute CMAKE.
start "" %CMAKE_PATH%\cmake-gui.exe -D"CMAKE_MAKE_PROGRAM:PATH=D:\Hobby\avr8-gnu-toolchain\bin"
start "" %ECLIPSE_PATH%\eclipse.exe

:: exit
