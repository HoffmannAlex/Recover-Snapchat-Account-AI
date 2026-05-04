# Recover-Snapchat-Account-AI Makefile
# C Implementation - Version 2026.1

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -I./src
LDFLAGS = -lcurl -lpthread -lm

# Platform detection
ifeq ($(OS),Windows_NT)
    TARGET = snapchat_security_test.exe
    LDFLAGS += -lws2_32
    RM = del /Q
    RMDIR = rmdir /S /Q
else
    TARGET = snapchat_security_test
    LDFLAGS += -lrt
    RM = rm -f
    RMDIR = rm -rf
endif

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

.PHONY: all clean directories install

all: directories $(BINDIR)/$(TARGET)

directories:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	@if not exist $(BINDIR) mkdir $(BINDIR)
	@if not exist logs mkdir logs

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RMDIR) $(OBJDIR) 2>nul || true
	$(RMDIR) $(BINDIR) 2>nul || true
	$(RM) *.exe 2>nul || true
	@echo "Clean complete"

install: all
	@echo "Installing $(TARGET)..."
ifeq ($(OS),Windows_NT)
	@copy $(BINDIR)\$(TARGET) . >nul
else
	@cp $(BINDIR)/$(TARGET) .
	@chmod +x $(TARGET)
endif
	@echo "Installation complete"

# Development helpers
debug: CFLAGS = -Wall -Wextra -g -O0 -std=c99 -I./src -DDEBUG
debug: all

release: CFLAGS = -Wall -Wextra -O3 -std=c99 -I./src -DNDEBUG
release: all

# Testing
test: all
	./$(BINDIR)/$(TARGET) --help

# Static analysis
analyze:
	cppcheck --enable=all --inconclusive --std=c99 $(SRCDIR)/

# Format code
format:
	clang-format -i $(SRCDIR)/*.c $(SRCDIR)/*.h
