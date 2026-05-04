#!/bin/bash
# Build script for Recover-Snapchat-Account-AI on Unix-like systems
# Supports Linux, macOS, and WSL

set -e

echo "Recover-Snapchat-Account-AI Build Script"
echo "========================================="
echo ""

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="Linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macOS"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    OS="Windows"
else
    OS="Unknown"
fi

echo "Detected OS: $OS"
echo ""

# Check for compiler
if command -v gcc &> /dev/null; then
    CC=gcc
    echo "Using GCC compiler"
elif command -v clang &> /dev/null; then
    CC=clang
    echo "Using Clang compiler"
else
    echo "ERROR: No C compiler found. Please install GCC or Clang."
    echo ""
    echo "For Linux (Debian/Ubuntu):"
    echo "  sudo apt-get install build-essential"
    echo ""
    echo "For macOS:"
    echo "  xcode-select --install"
    echo "  or: brew install gcc"
    exit 1
fi

# Check for curl
if ! pkg-config --exists libcurl 2>/dev/null; then
    echo "WARNING: libcurl not found via pkg-config"
    echo "Attempting to compile anyway..."
    echo ""
    echo "If build fails, install libcurl:"
    echo "  Linux: sudo apt-get install libcurl4-openssl-dev"
    echo "  macOS: brew install curl"
fi

# Create directories
mkdir -p obj bin logs

echo ""
echo "Compiling source files..."
echo ""

CFLAGS="-Wall -Wextra -O2 -std=c99 -I./src"
if [[ "$OS" == "macOS" ]]; then
    CFLAGS="$CFLAGS -I/usr/local/opt/curl/include"
fi

# Compile each source file
$CC $CFLAGS -c src/logging.c -o obj/logging.o
$CC $CFLAGS -c src/password_generator.c -o obj/password_generator.o
$CC $CFLAGS -c src/request_manager.c -o obj/request_manager.o
$CC $CFLAGS -c src/proxy_manager.c -o obj/proxy_manager.o
$CC $CFLAGS -c src/tor_manager.c -o obj/tor_manager.o
$CC $CFLAGS -c src/csrf_manager.c -o obj/csrf_manager.o
$CC $CFLAGS -c src/monitoring.c -o obj/monitoring.o
$CC $CFLAGS -c src/security_tester.c -o obj/security_tester.o
$CC $CFLAGS -c src/main.c -o obj/main.o

echo ""
echo "Linking..."
echo ""

LDFLAGS="-lcurl -lpthread -lm"
if [[ "$OS" == "macOS" ]]; then
    LDFLAGS="$LDFLAGS -L/usr/local/opt/curl/lib"
fi

$CC obj/logging.o obj/password_generator.o obj/request_manager.o obj/proxy_manager.o \
    obj/tor_manager.o obj/csrf_manager.o obj/monitoring.o obj/security_tester.o \
    obj/main.o -o bin/snapchat_security_test $LDFLAGS

echo ""
echo "========================================="
echo "Build successful!"
echo "========================================="
echo ""
echo "Executable: bin/snapchat_security_test"
echo ""
echo "To test installation:"
echo "  ./bin/snapchat_security_test --help"
echo ""
echo "To install to current directory:"
echo "  cp bin/snapchat_security_test ."
echo ""
