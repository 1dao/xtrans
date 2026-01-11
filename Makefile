# Makefile for xtrans translation tool (Linux)
# 编译模式：默认 Release，支持 make debug 切换调试版
BUILD_TYPE ?= release
CC = gcc
INCLUDES = -I. -Imbedtls/include
LIBS = 
# 目录配置
OBJ_DIR = .obj
TARGET = xtrans
MBEDTLS_LIB_DIR = mbedtls/library

# 批量获取所有源文件（无需逐个罗列）
MAIN_SRC = $(wildcard *.c)  # 当前目录所有.c（xtrans.c、xhttpc.c）
MBEDTLS_SRC = $(wildcard $(MBEDTLS_LIB_DIR)/*.c)  # mbedtls所有.c文件
ALL_SRC = $(MAIN_SRC) $(MBEDTLS_SRC)

# 批量生成.obj文件路径（按编译模式区分目录）
OBJS = \
    $(patsubst %.c, $(OBJ_DIR)/$(BUILD_TYPE)/%.o, $(MAIN_SRC)) \
    $(patsubst $(MBEDTLS_LIB_DIR)/%.c, $(OBJ_DIR)/$(BUILD_TYPE)/$(MBEDTLS_LIB_DIR)/%.o, $(MBEDTLS_SRC))

# 编译选项（按模式切换）
ifeq ($(BUILD_TYPE), debug)
    CFLAGS = -Wall -Wextra -O0 -g -std=c99  # Debug：无优化+调试信息
else
    CFLAGS = -Wall -Wextra -O3 -std=c99 -DNDEBUG -Wno-unused-function  # Release：最高优化
endif

# 默认目标（Release版）
all: $(TARGET)
	@echo "Build completed: $(TARGET) (Release mode)"

# 调试版目标
debug:
	@$(MAKE) BUILD_TYPE=debug  # 调用自身切换模式
	@echo "Build completed: $(TARGET) (Debug mode)"

# 链接：批量匹配.obj文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@rm -rf $(OBJ_DIR)/$(BUILD_TYPE)  # 编译完成删除中间目录

# 编译规则：自动创建目录+批量编译（修复隐式规则语法）
$(OBJ_DIR)/$(BUILD_TYPE)/%.o: %.c
	@mkdir -p $(dir $@)  # 递归创建目标目录
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理所有产物
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Clean completed"

# 重新编译
rebuild: clean all

# 帮助信息
help:
	@echo "Usage:"
	@echo "  make                - Build Release version (O3优化)"
	@echo "  make debug          - Build Debug version (无优化+调试信息)"
	@echo "  make clean          - Clean all files"
	@echo "  make rebuild        - Clean and rebuild Release"

.PHONY: all debug clean rebuild help