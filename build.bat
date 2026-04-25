@echo off
chcp 65001 > nul
title xtrans-翻译小工具（x64）
setlocal enabledelayedexpansion

:: 定义颜色常量
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "RESET=[0m"

:: ======================================
:: 步骤1：初始化 VS2022 x64 编译环境
:: ======================================
REM Build script for xtrans translation tool (Windows - cl compiler direct)
REM Usage: build_cl.bat [clean|debug]
REM   clean  - 清理编译产物
REM   debug  - 编译Debug版本（默认Release）

REM Try multiple VS installation paths
REM set "VS_VCVARS="
for %%P in (
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
    "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "C:\Program Files\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "C:\software\MicrosoftVisual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "C:\software\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "C:\software\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
    "D:\software\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "D:\software\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
    "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "D:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
    "D:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "D:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "D:\Program Files\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
    "D:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
) do (
    if exist "%%P" (
        echo %GREEN%[MSVC Path:]%RESET% %%P
        set "VS_VCVARS=%%P"
        goto :found_vs
    )
)

:found_vs
if not defined VS_VCVARS (
    echo %RED%[ERROR]%RESET% Visual Studio not found in common locations
    echo %RED%[ERORO]%RESET% Please install Visual Studio or run from Developer Command Prompt
    pause
    exit /b 1
)

call !VS_VCVARS! x64

REM 检查是否在 Visual Studio 开发环境中
where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo %RED%[Error]%RESET% cl.exe not found in PATH
    echo %YELLOW%[Warn]%RESET% Please run this script from "Developer Command Prompt for VS" or "x64 Native Tools Command Prompt"
    echo %YELLOW%[Warn]%RESET% Or set up Visual Studio environment first:
    echo   "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    exit /b 1
)

REM 清理逻辑：删除.obj目录和最终可执行文件
if "%1"=="clean" (
    echo Cleaning...
    rd /S /Q .obj 2>nul
    del /Q xtrans.exe 2>nul
    del /Q xtrans.pdb 2>nul  REM 清理Debug版的pdb文件
    echo Clean completed.
    exit /b 0
)

REM ===== 编译模式区分：默认Release，指定debug则为Debug =====
set "BUILD_MODE=release"
set "CFLAGS=/W3 /TC /utf-8 /D_CRT_SECURE_NO_WARNINGS /I. /I"mbedtls\include" /c"

if "%1"=="debug" (
    set "BUILD_MODE=debug"
    REM Debug模式：关闭优化、启用调试信息、定义DEBUG宏
    set "CFLAGS=!CFLAGS! /Od /Zi /DDEBUG"
    echo Building xtrans [DEBUG] with cl compiler...
) else (
    REM Release模式：O2优化
    set "CFLAGS=!CFLAGS! /O2"
    echo Building xtrans [RELEASE] with cl compiler...
)
echo.

REM 创建.obj目录（包括mbedtls子目录）
echo %GREEN%[INFO]%RESET% Creating .obj directory...
md .obj 2>nul
md .obj\mbedtls 2>nul
md .obj\mbedtls\library 2>nul

REM ===== 批量编译主程序.c文件（当前目录下的xtrans.c、xhttpc.c，或直接*.c）=====
echo %GREEN%[INFO]%RESET% Compiling main files...
cl %CFLAGS% /Fo.obj\ xargs.c xtrans_google.c xtrans_bing.c xtrans.c xhttpc.c
REM 如果要批量匹配当前目录所有.c，替换为：
REM cl %CFLAGS% /Fo.obj\ *.c
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    rd /S /Q .obj 2>nul
    exit /b 1
)

REM ===== 批量编译mbedtls所有.c文件（类Linux的*.c方式）=====
echo %GREEN%[INFO]%RESET% Compiling mbedtls library files...
cl %CFLAGS% /Fo.obj\mbedtls\library\ mbedtls\library\*.c
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    rd /S /Q .obj 2>nul
    exit /b 1
)

REM ===== 批量链接所有.obj文件（无需逐个罗列）=====
echo %GREEN%[INFO]%RESET% Linking...
cl /Fe:xtrans.exe ^
    .obj\*.obj ^
    .obj\mbedtls\library\*.obj ^
    ws2_32.lib advapi32.lib

if %ERRORLEVEL% neq 0 (
    echo Linking failed!
    rd /S /Q .obj 2>nul
    exit /b 1
)

REM 编译完成后删除.obj目录
echo %GREEN%[INFO]%RESET% Cleaning up .obj directory...
rd /S /Q .obj 2>nul

echo.
echo Build completed: xtrans.exe (Mode: %BUILD_MODE%)
endlocal
