:: This script setup the build enviornment for ARM and AVR.
:: NOTE: Please set the required paths below before executing 
::       this script.
@echo off

:: Populate paths.
set ARM_TOOLSET=C:\Program Files (x86)\GNU Tools Arm Embedded\7 2018-q2-update\bin\
set AVR_TOOLSET=C:\private\avr8-gnu-toolchain\bin\
set OPENOCD=C:\private\openocd-20190210\OpenOCD-20190210-0.10.0\bin
set PYTHON_PATH=D:\Hobby\python\
set PYTHON_SCRIPTS_PATH=D:\Hobby\python\Scripts
set MAKE_PATH=C:\private\GnuWin32\bin
set CMAKE_PATH="C:\Program Files\CMake\bin\"
set ECLIPSE_PATH="C:\private\eclipse"

:: Add ARM toolchain.
set PATH=%AVR_TOOLSET%;%ARM_TOOLSET%;%OPENOCD%;%MAKE_PATH%;%PYTHON_PATH%;%PYTHON_SCRIPTS_PATH%;%PATH%

:: Execute CMAKE.
start "" %CMAKE_PATH%\cmake-gui.exe
start "" %ECLIPSE_PATH%\eclipse.exe

:: exit
