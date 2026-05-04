@echo off
REM Build script for Recover-Snapchat-Account-AI on Windows
REM Requires MinGW-w64 or MSVC with curl library

echo Recover-Snapchat-Account-AI Build Script

echo.
echo Checking for compiler...

where gcc >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Found GCC compiler
    set COMPILER=gcc
    goto build_gcc
)

where cl >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Found MSVC compiler
    set COMPILER=msvc
    goto build_msvc
)

echo ERROR: No compiler found. Please install MinGW-w64 or Visual Studio.
echo.
echo For MinGW-w64:
echo   1. Download from https://mingw-w64.org/
echo   2. Add bin folder to PATH
echo.
echo For Visual Studio:
echo   1. Install Visual Studio with C++ workload
echo   2. Run from Developer Command Prompt
goto end

:build_gcc
echo.
echo Building with GCC...
echo.

if not exist obj mkdir obj
if not exist bin mkdir bin
if not exist logs mkdir logs

gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/logging.c -o obj/logging.o
gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/password_generator.c -o obj/password_generator.o
gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/request_manager.c -o obj/request_manager.o
gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/proxy_manager.c -o obj/proxy_manager.o
gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/tor_manager.c -o obj/tor_manager.o
gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/csrf_manager.c -o obj/csrf_manager.o
gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/monitoring.c -o obj/monitoring.o
gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/security_tester.c -o obj/security_tester.o
gcc -c -Wall -Wextra -O2 -std=c99 -I./src src/main.c -o obj/main.o

echo.
echo Linking...
gcc obj/logging.o obj/password_generator.o obj/request_manager.o obj/proxy_manager.o obj/tor_manager.o obj/csrf_manager.o obj/monitoring.o obj/security_tester.o obj/main.o -o bin/snapchat_security_test.exe -lcurl -lpthread -lm -lws2_32

if %ERRORLEVEL% == 0 (
    echo.
    echo Build successful!
    echo Executable: bin\snapchat_security_test.exe
    echo.
    echo To run: bin\snapchat_security_test.exe --help
) else (
    echo.
    echo Build failed!
    echo Make sure libcurl is installed:
    echo   - vcpkg install curl
    echo   - Or download from https://curl.se/windows/
)
goto end

:build_msvc
echo.
echo Building with MSVC...
echo.

if not exist obj mkdir obj
if not exist bin mkdir bin
if not exist logs mkdir logs

cl /c /W4 /O2 /TC /I./src src/logging.c /Foobj/logging.obj
cl /c /W4 /O2 /TC /I./src src/password_generator.c /Foobj/password_generator.obj
cl /c /W4 /O2 /TC /I./src src/request_manager.c /Foobj/request_manager.obj
cl /c /W4 /O2 /TC /I./src src/proxy_manager.c /Foobj/proxy_manager.obj
cl /c /W4 /O2 /TC /I./src src/tor_manager.c /Foobj/tor_manager.obj
cl /c /W4 /O2 /TC /I./src src/csrf_manager.c /Foobj/csrf_manager.obj
cl /c /W4 /O2 /TC /I./src src/monitoring.c /Foobj/monitoring.obj
cl /c /W4 /O2 /TC /I./src src/security_tester.c /Foobj/security_tester.obj
cl /c /W4 /O2 /TC /I./src src/main.c /Foobj/main.obj

echo.
echo Linking...
link obj/logging.obj obj/password_generator.obj obj/request_manager.obj obj/proxy_manager.obj obj/tor_manager.obj obj/csrf_manager.obj obj/monitoring.obj obj/security_tester.obj obj/main.obj /OUT:bin\snapchat_security_test.exe libcurl.lib ws2_32.lib

if %ERRORLEVEL% == 0 (
    echo.
    echo Build successful!
    echo Executable: bin\snapchat_security_test.exe
) else (
    echo.
    echo Build failed!
    echo Make sure libcurl is available in your library path.
)
goto end

:end
echo.
pause
