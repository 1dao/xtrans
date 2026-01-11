@echo off
REM Build script for xtrans translation tool (Windows - cl compiler direct)
REM Usage: build_cl.bat [clean]

setlocal enabledelayedexpansion
call "C:\software\MicrosoftVisual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM 检查是否在 Visual Studio 开发环境中
where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: cl.exe not found in PATH
    echo Please run this script from "Developer Command Prompt for VS" or "x64 Native Tools Command Prompt"
    echo Or set up Visual Studio environment first:
    echo   "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    exit /b 1
)

REM 清理逻辑：删除.obj目录和最终可执行文件
if "%1"=="clean" (
    echo Cleaning...
    rd /S /Q .obj 2>nul
    del /Q xtrans.exe 2>nul
    echo Clean completed.
    exit /b 0
)

REM 创建.obj目录（包括mbedtls子目录）
echo Creating .obj directory...
md .obj 2>nul
md .obj\mbedtls 2>nul
md .obj\mbedtls\library 2>nul

echo Building xtrans with cl compiler...
echo.

REM 编译选项：添加/Fo指定.obj输出目录，统一存放编译产物
set CFLAGS=/W3 /O2 /TC /utf-8 /D_CRT_SECURE_NO_WARNINGS /I. /I"mbedtls\include" /c

REM 编译主程序文件（输出到.obj根目录）
echo Compiling main files...
cl %CFLAGS% /Fo.obj\ xtrans.c xhttpc.c
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    rd /S /Q .obj 2>nul
    exit /b 1
)

REM 编译 mbedtls crypto 文件（输出到.obj/mbedtls/library）
echo Compiling mbedtls crypto files...
cl %CFLAGS% /Fo.obj\mbedtls\library\ mbedtls\library\aes.c mbedtls\library\aesni.c mbedtls\library\arc4.c mbedtls\library\aria.c ^
    mbedtls\library\asn1parse.c mbedtls\library\asn1write.c mbedtls\library\base64.c mbedtls\library\bignum.c ^
    mbedtls\library\blowfish.c mbedtls\library\camellia.c mbedtls\library\ccm.c mbedtls\library\chacha20.c ^
    mbedtls\library\chachapoly.c mbedtls\library\cipher.c mbedtls\library\cipher_wrap.c mbedtls\library\cmac.c ^
    mbedtls\library\ctr_drbg.c mbedtls\library\des.c mbedtls\library\dhm.c mbedtls\library\ecdh.c ^
    mbedtls\library\ecdsa.c mbedtls\library\ecjpake.c mbedtls\library\ecp.c mbedtls\library\ecp_curves.c ^
    mbedtls\library\entropy.c mbedtls\library\entropy_poll.c mbedtls\library\error.c mbedtls\library\gcm.c ^
    mbedtls\library\havege.c mbedtls\library\hkdf.c mbedtls\library\hmac_drbg.c mbedtls\library\md.c ^
    mbedtls\library\md2.c mbedtls\library\md4.c mbedtls\library\md5.c mbedtls\library\md_wrap.c ^
    mbedtls\library\memory_buffer_alloc.c mbedtls\library\nist_kw.c mbedtls\library\oid.c ^
    mbedtls\library\padlock.c mbedtls\library\pem.c mbedtls\library\pk.c mbedtls\library\pk_wrap.c ^
    mbedtls\library\pkcs12.c mbedtls\library\pkcs5.c mbedtls\library\pkparse.c mbedtls\library\pkwrite.c ^
    mbedtls\library\platform.c mbedtls\library\platform_util.c mbedtls\library\poly1305.c ^
    mbedtls\library\ripemd160.c mbedtls\library\rsa.c mbedtls\library\rsa_internal.c mbedtls\library\sha1.c ^
    mbedtls\library\sha256.c mbedtls\library\sha512.c mbedtls\library\threading.c mbedtls\library\timing.c ^
    mbedtls\library\version.c mbedtls\library\version_features.c mbedtls\library\xtea.c
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    rd /S /Q .obj 2>nul
    exit /b 1
)

REM 编译 mbedtls x509 文件（输出到.obj/mbedtls/library）
echo Compiling mbedtls x509 files...
cl %CFLAGS% /Fo.obj\mbedtls\library\ mbedtls\library\certs.c mbedtls\library\pkcs11.c mbedtls\library\x509.c ^
    mbedtls\library\x509_create.c mbedtls\library\x509_crl.c mbedtls\library\x509_crt.c ^
    mbedtls\library\x509_csr.c mbedtls\library\x509write_crt.c mbedtls\library\x509write_csr.c
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    rd /S /Q .obj 2>nul
    exit /b 1
)

REM 编译 mbedtls tls 文件（输出到.obj/mbedtls/library）
echo Compiling mbedtls tls files...
cl %CFLAGS% /Fo.obj\mbedtls\library\ mbedtls\library\debug.c mbedtls\library\net_sockets.c mbedtls\library\ssl_cache.c ^
    mbedtls\library\ssl_ciphersuites.c mbedtls\library\ssl_cli.c mbedtls\library\ssl_cookie.c ^
    mbedtls\library\ssl_srv.c mbedtls\library\ssl_ticket.c mbedtls\library\ssl_tls.c
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    rd /S /Q .obj 2>nul
    exit /b 1
)

REM 链接：添加 advapi32.lib 解决 CryptoAPI 符号未解析问题，引用.obj目录下的所有目标文件
echo Linking...
cl /Fe:xtrans.exe ^
    .obj\xtrans.obj .obj\xhttpc.obj ^
    .obj\mbedtls\library\aes.obj .obj\mbedtls\library\aesni.obj .obj\mbedtls\library\arc4.obj .obj\mbedtls\library\aria.obj ^
    .obj\mbedtls\library\asn1parse.obj .obj\mbedtls\library\asn1write.obj .obj\mbedtls\library\base64.obj .obj\mbedtls\library\bignum.obj ^
    .obj\mbedtls\library\blowfish.obj .obj\mbedtls\library\camellia.obj .obj\mbedtls\library\ccm.obj .obj\mbedtls\library\chacha20.obj ^
    .obj\mbedtls\library\chachapoly.obj .obj\mbedtls\library\cipher.obj .obj\mbedtls\library\cipher_wrap.obj .obj\mbedtls\library\cmac.obj ^
    .obj\mbedtls\library\ctr_drbg.obj .obj\mbedtls\library\des.obj .obj\mbedtls\library\dhm.obj .obj\mbedtls\library\ecdh.obj ^
    .obj\mbedtls\library\ecdsa.obj .obj\mbedtls\library\ecjpake.obj .obj\mbedtls\library\ecp.obj .obj\mbedtls\library\ecp_curves.obj ^
    .obj\mbedtls\library\entropy.obj .obj\mbedtls\library\entropy_poll.obj .obj\mbedtls\library\error.obj .obj\mbedtls\library\gcm.obj ^
    .obj\mbedtls\library\havege.obj .obj\mbedtls\library\hkdf.obj .obj\mbedtls\library\hmac_drbg.obj .obj\mbedtls\library\md.obj ^
    .obj\mbedtls\library\md2.obj .obj\mbedtls\library\md4.obj .obj\mbedtls\library\md5.obj .obj\mbedtls\library\md_wrap.obj ^
    .obj\mbedtls\library\memory_buffer_alloc.obj .obj\mbedtls\library\nist_kw.obj .obj\mbedtls\library\oid.obj ^
    .obj\mbedtls\library\padlock.obj .obj\mbedtls\library\pem.obj .obj\mbedtls\library\pk.obj .obj\mbedtls\library\pk_wrap.obj ^
    .obj\mbedtls\library\pkcs12.obj .obj\mbedtls\library\pkcs5.obj .obj\mbedtls\library\pkparse.obj .obj\mbedtls\library\pkwrite.obj ^
    .obj\mbedtls\library\platform.obj .obj\mbedtls\library\platform_util.obj .obj\mbedtls\library\poly1305.obj ^
    .obj\mbedtls\library\ripemd160.obj .obj\mbedtls\library\rsa.obj .obj\mbedtls\library\rsa_internal.obj .obj\mbedtls\library\sha1.obj ^
    .obj\mbedtls\library\sha256.obj .obj\mbedtls\library\sha512.obj .obj\mbedtls\library\threading.obj .obj\mbedtls\library\timing.obj ^
    .obj\mbedtls\library\version.obj .obj\mbedtls\library\version_features.obj .obj\mbedtls\library\xtea.obj ^
    .obj\mbedtls\library\certs.obj .obj\mbedtls\library\pkcs11.obj .obj\mbedtls\library\x509.obj ^
    .obj\mbedtls\library\x509_create.obj .obj\mbedtls\library\x509_crl.obj .obj\mbedtls\library\x509_crt.obj ^
    .obj\mbedtls\library\x509_csr.obj .obj\mbedtls\library\x509write_crt.obj .obj\mbedtls\library\x509write_csr.obj ^
    .obj\mbedtls\library\debug.obj .obj\mbedtls\library\net_sockets.obj .obj\mbedtls\library\ssl_cache.obj ^
    .obj\mbedtls\library\ssl_ciphersuites.obj .obj\mbedtls\library\ssl_cli.obj .obj\mbedtls\library\ssl_cookie.obj ^
    .obj\mbedtls\library\ssl_srv.obj .obj\mbedtls\library\ssl_ticket.obj .obj\mbedtls\library\ssl_tls.obj ^
    ws2_32.lib advapi32.lib  

if %ERRORLEVEL% neq 0 (
    echo Linking failed!
    rd /S /Q .obj 2>nul
    exit /b 1
)

REM 编译完成后删除.obj目录
echo Cleaning up .obj directory...
rd /S /Q .obj 2>nul

echo.
echo Build completed: xtrans.exe
endlocal