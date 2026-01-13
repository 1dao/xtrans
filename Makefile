# Makefile for xtrans translation tool (Linux/Windows)
# 编译模式：默认 Release，支持 make debug 切换调试版
BUILD_TYPE ?= tiny

# 检测操作系统和编译器环境
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)
GCC_MACHINE := $(shell gcc -dumpmachine 2>/dev/null || echo unknown)
GCC_VERSION := $(shell gcc --version 2>/dev/null | head -1 || echo "not found")

# 检测是否为MINGW64环境
ifneq ($(findstring MINGW,$(GCC_MACHINE)),)
    # MINGW64环境
    CC = gcc
    PLATFORM = MINGW64
    EXE_EXT = .exe
    LIBS = -lws2_32
    $(info Detected MINGW64 environment: $(GCC_MACHINE))
    $(info GCC version: $(GCC_VERSION))
else ifeq ($(OS),Windows_NT)
    # Windows环境但检测GCC
    ifeq ($(findstring not found,$(GCC_VERSION)),)
        # 找到GCC但不是MINGW64
        CC = gcc
        PLATFORM = Windows
        EXE_EXT = .exe
    LIBS = -lws2_32 -ladvapi32 -lcrypt32
        $(info Detected Windows environment with GCC: $(GCC_MACHINE))
        $(info GCC version: $(GCC_VERSION))
    else
        # Windows环境且没有找到GCC
        $(info Windows detected but GCC not found)
        $(info Please install MINGW64 development tools:)
        $(info 1. Download from: https://www.mingw-w64.org/downloads/)
        $(info 2. Or use MSYS2: pacman -S mingw-w64-x86_64-toolchain)
        $(info 3. Ensure /mingw64/bin is in your PATH)
        $(error GCC compiler not found. Please install MINGW64 development tools.)
    endif
else
    # Linux环境
    CC = gcc
    PLATFORM = Linux
    EXE_EXT =
    LIBS =
    $(info Detected Linux environment)
    $(info GCC version: $(GCC_VERSION))
endif

INCLUDES = -I. -Imbedtls/include
# 目录配置
OBJ_DIR = .obj
TARGET = xtrans$(EXE_EXT)
MBEDTLS_LIB_DIR = mbedtls/library

# 批量获取所有源文件（无需逐个罗列）
MAIN_SRC = $(wildcard *.c)  # 当前目录所有.c（xtrans.c、xhttpc.c）
MBEDTLS_SRC = $(wildcard $(MBEDTLS_LIB_DIR)/*.c)  # mbedtls所有.c文件
ALL_SRC = $(MAIN_SRC) $(MBEDTLS_SRC)

# 批量生成.obj文件路径（按编译模式区分目录）
OBJS = \
    $(patsubst %.c, $(OBJ_DIR)/$(BUILD_TYPE)/%.o, $(MAIN_SRC)) \
    $(patsubst $(MBEDTLS_LIB_DIR)/%.c, $(OBJ_DIR)/$(BUILD_TYPE)/$(MBEDTLS_LIB_DIR)/%.o, $(MBEDTLS_SRC))

# 编译选项（按模式切换和平台调整）
ifeq ($(BUILD_TYPE), debug)
    ifeq ($(PLATFORM),MINGW64)
        CFLAGS = -Wall -Wextra -O0 -g -std=c11 -D__USE_MINGW_ANSI_STDIO=1 -DWIN32_LEAN_AND_MEAN -Wno-unknown-pragmas -Wno-sign-compare
    else
        CFLAGS = -Wall -Wextra -O0 -g -std=c11  # Linux Debug
    endif
else ifeq ($(BUILD_TYPE), tiny)
    ifeq ($(PLATFORM),MINGW64)
        CFLAGS = -Os -std=c11 -DNDEBUG -D__USE_MINGW_ANSI_STDIO=1 -DWIN32_LEAN_AND_MEAN -DWINVER=0x0601 -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -fno-ident -s
    else
        CFLAGS = -Os -std=c11 -DNDEBUG -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -fno-ident -s
    endif
else
    ifeq ($(PLATFORM),MINGW64)
        CFLAGS = -Wall -Os -std=c11 -DNDEBUG -D__USE_MINGW_ANSI_STDIO=1 -DWIN32_LEAN_AND_MEAN -DWINVER=0x0601 -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -s
    else
        CFLAGS = -Wall -Wextra -O3 -std=c11 -DNDEBUG -Wno-unused-function  # Linux Release
    endif
endif

# 默认目标（Tiny版）
all: $(TARGET)
	@echo "Build completed: $(TARGET) (Tiny mode) for $(PLATFORM)"

# 调试版目标
debug:
	@$(MAKE) BUILD_TYPE=debug  # 调用自身切换模式
	@echo "Build completed: $(TARGET) (Debug mode) for $(PLATFORM)"

# Release版本目标
release:
	@$(MAKE) BUILD_TYPE=release  # 调用自身切换模式
	@echo "Build completed: $(TARGET) (Release mode) for $(PLATFORM)"

# 最小体积版本目标
tiny:
	@$(MAKE) BUILD_TYPE=tiny  # 调用自身切换模式
	@echo "Build completed: $(TARGET) (Tiny mode) for $(PLATFORM)"

# 链接：批量匹配.obj文件
$(TARGET): $(OBJS)
ifeq ($(PLATFORM),MINGW64)
ifeq ($(BUILD_TYPE), tiny)
	$(CC) $(CFLAGS) -Wl,--gc-sections,--strip-all,--subsystem,console,--compress-debug-sections,--gc-sections -static-libgcc -o $@ $^ $(LIBS)
else
	$(CC) $(CFLAGS) -Wl,--gc-sections,--strip-all,--subsystem,console -static-libgcc -o $@ $^ $(LIBS)
endif
else
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
endif
	@rm -rf $(OBJ_DIR)/$(BUILD_TYPE)  # 编译完成删除中间目录

# 编译规则：自动创建目录+批量编译（修复隐式规则语法）
$(OBJ_DIR)/$(BUILD_TYPE)/%.o: %.c
	@mkdir -p $(dir $@)  # 递归创建目标目录
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理所有产物
clean:
ifeq ($(PLATFORM),MINGW64)
	-rm -rf $(OBJ_DIR) $(TARGET) 2>/dev/null || del /Q /F /S $(OBJ_DIR) 2>/dev/null || rmdir /S /Q $(OBJ_DIR) 2>/dev/null
	-del /Q /F $(TARGET) 2>/dev/null
else
	rm -rf $(OBJ_DIR) $(TARGET)
endif
	@echo "Clean completed"

# 重新编译
rebuild: clean all

# 帮助信息
help:
	@echo "Usage:"
	@echo "  make                - Build Tiny version (最小体积 - 默认)"
	@echo "  make release        - Build Release version (平衡版本)"
	@echo "  make debug          - Build Debug version (无优化+调试信息)"
	@echo "  make clean          - Clean all files"
	@echo "  make rebuild        - Clean and rebuild Tiny"
	@echo ""
	@echo "Current Platform: $(PLATFORM)"
	@echo "Target executable: $(TARGET)"
ifeq ($(PLATFORM),MINGW64)
	@echo "MINGW64 environment detected - Windows build supported"
	@echo ""
	@echo "编译选项说明："
	@echo "  Tiny:     最小体积 (默认)"
	@echo "  Release:  平衡性能和体积"
	@echo "  Debug:    包含调试信息，无优化"
endif

.PHONY: all debug clean rebuild help
