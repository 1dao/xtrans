# Makefile for xtrans translation tool
# 简化版本，直接编译

# 检测操作系统
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)
GCC_MACHINE := $(shell gcc -dumpmachine 2>/dev/null || echo unknown)
GCC_VERSION := $(shell gcc --version 2>/dev/null | head -1 || echo "not found")

# 平台检测
ifneq ($(findstring MINGW,$(UNAME_S)),)
    # MINGW64环境
    CC = gcc
    PLATFORM = MINGW64
    EXE_EXT = .exe
    LIBS = -lws2_32
    $(info Detected MINGW64 environment: $(GCC_MACHINE))
else ifeq ($(OS),Windows_NT)
    # Windows环境但检测GCC
    ifeq ($(findstring not found,$(GCC_VERSION)),)
        # 找到GCC但不是MINGW64
        CC = gcc
        PLATFORM = Windows
        EXE_EXT = .exe
        LIBS = -lws2_32 -ladvapi32 -lcrypt32
        $(info Detected Windows environment with GCC: $(GCC_MACHINE))
    else
        # Windows环境且没有找到GCC
        $(error GCC compiler not found. Please install MINGW64 development tools.)
    endif
else
    # Linux环境
    CC = gcc
    PLATFORM = Linux
    EXE_EXT =
    LIBS =
    $(info Detected Linux environment)
endif

# 源文件
SOURCES = xtrans.c xhttpc.c
TARGET = xtrans$(EXE_EXT)
MBEDTLS_DIR = mbedtls/library
LIBDIR = .libs
OBJDIR = .obj
INCLUDES = -I. -Imbedtls/include

# 编译选项
CFLAGS = -Wall -O2 -std=c11 -DNDEBUG $(INCLUDES)
LDFLAGS = -L$(LIBDIR) -lmbedtls -lmbedx509 -lmbedcrypto $(LIBS)

# 对象文件
OBJECTS = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

# 默认编译
default: all

# 检查mbedtls/library/Makefile是否存在，不存在则复制
mbedtls-check:
	@if [ ! -f "$(MBEDTLS_DIR)/Makefile" ]; then \
		echo "Copying Makefile.mbedtls.mk to $(MBEDTLS_DIR)/Makefile"; \
		cp Makefile.mbedtls.mk $(MBEDTLS_DIR)/Makefile; \
	fi

# 默认目标
.PHONY: all debug release tiny clean rebuild help test

all: mbedtls-check $(TARGET) clean-all-obj
        @echo "Build completed: $(TARGET)"

$(TARGET): $(OBJECTS) mbedtls-libs
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# 编译对象文件规则
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@echo "  CC    $<"
	$(CC) $(CFLAGS) -c $< -o $@

# 创建对象目录
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# 清理对象目录
clean-obj:
	@rm -rf $(OBJDIR) 2>/dev/null || true

# 清理所有对象文件（包括mbedtls）
clean-all-obj:
	@rm -rf $(OBJDIR) 2>/dev/null || true
	@cd $(MBEDTLS_DIR) && $(MAKE) clean 2>/dev/null || true

mbedtls-libs: $(LIBDIR)/libmbedtls.a $(LIBDIR)/libmbedx509.a $(LIBDIR)/libmbedcrypto.a

$(LIBDIR)/libmbedtls.a $(LIBDIR)/libmbedx509.a $(LIBDIR)/libmbedcrypto.a: | $(LIBDIR)
	@echo "Building mbedtls library..."
	@cd $(MBEDTLS_DIR) && CFLAGS="-fPIC -O2" $(MAKE) -j4
	@echo "Moving mbedtls libraries to $(LIBDIR)..."
	@mv $(MBEDTLS_DIR)/libmbedtls.a $(MBEDTLS_DIR)/libmbedx509.a $(MBEDTLS_DIR)/libmbedcrypto.a $(LIBDIR)/

# 创建库目录
$(LIBDIR):
	@mkdir -p $(LIBDIR)

debug: CFLAGS += -g -O0 -DDEBUG
debug: $(TARGET) clean-all-obj
	@echo "Debug build completed: $(TARGET)"

release: CFLAGS += -O3
release: $(TARGET) clean-all-obj
	@echo "Release build completed: $(TARGET)"

tiny: CFLAGS += -Os -ffunction-sections -fdata-sections
tiny: LDFLAGS += -Wl,--gc-sections
tiny: $(TARGET) clean-all-obj
	@echo "Tiny build completed: $(TARGET)"

# 编译mbedtls
mbedtls:
	@echo "Building mbedtls library..."
	@cd $(MBEDTLS_DIR) && $(MAKE) clean && CFLAGS="-fPIC -O2" $(MAKE) -j4
	@echo "Mbedtls build completed"

# 清理
clean:
	@echo "Cleaning..."
	@rm -f $(TARGET) $(TARGET).exe 2>/dev/null || true
	@rm -rf $(OBJDIR) 2>/dev/null || true
	@rm -rf $(LIBDIR) 2>/dev/null || true
	@echo "Clean completed"

# 重新编译
rebuild: clean all
	@echo "Rebuild completed"

# 测试
test: $(TARGET)
	@echo "Running translation tests..."
	@echo "Testing: Hello world"
	@./$(TARGET) "Hello world"
	@echo "Testing: Chinese to English"
	@./$(TARGET) "你好世界" -t en
	@echo "All tests completed successfully!"

# 帮助
help:
	@echo "xtrans Translation Tool Build System"
	@echo "================================="
	@echo ""
	@echo "Available targets:"
	@echo "  make        - Build (default, optimized)"
	@echo "  make debug  - Build debug version"
	@echo "  make release- Build release version"
	@echo "  make tiny   - Build tiny version"
	@echo "  make mbedtls - Build mbedtls library"
	@echo "  make test   - Build and test"
	@echo "  make clean   - Clean build files"
	@echo "  make rebuild- Clean and rebuild"
	@echo ""
	@echo "Current Configuration:"
	@echo "  Platform: $(PLATFORM)"
	@echo "  Target: $(TARGET)"
	@echo "  Compiler: $(CC)"
