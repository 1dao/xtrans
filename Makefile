# Makefile for xtrans translation tool (Linux)
# 编译模式配置：默认 debug，支持 BUILD_TYPE=release 或直接 make release
BUILD_TYPE ?= debug  # 定义默认编译模式（?= 表示未指定时生效）

# 编译器设置（根据编译模式切换选项）
CC = gcc
INCLUDES = -I. -Imbedtls/include
LIBS = 
#LIBS = -liconv

# 目录设置
MBEDTLS_LIB_DIR = mbedtls/library
MBEDTLS_INC_DIR = mbedtls/include
OBJ_DIR = .obj  # 中间文件目录（统一存放.o文件）
TARGET = xtrans  # 目标可执行文件（release版可改为xtrans_release，按需调整）

# 根据 BUILD_TYPE 配置编译选项
ifeq ($(BUILD_TYPE), release)
    # Release 版：开启最高优化、关闭调试、忽略无关警告
    CFLAGS = -Wall -Wextra -O3 -std=c99 -DNDEBUG -Wno-unused-function -Wno-unused-variable
else
    # Debug 版（默认）：无优化、保留调试信息、全量警告
    CFLAGS = -Wall -Wextra -O0 -g -std=c99
endif

# 源文件（保持原有分类）
XTRANS_SRC = xtrans.c
XHTTPC_SRC = xhttpc.c
MBEDTLS_CRYPTO_SRC = \
	$(MBEDTLS_LIB_DIR)/aes.c \
	$(MBEDTLS_LIB_DIR)/aesni.c \
	$(MBEDTLS_LIB_DIR)/arc4.c \
	$(MBEDTLS_LIB_DIR)/aria.c \
	$(MBEDTLS_LIB_DIR)/asn1parse.c \
	$(MBEDTLS_LIB_DIR)/asn1write.c \
	$(MBEDTLS_LIB_DIR)/base64.c \
	$(MBEDTLS_LIB_DIR)/bignum.c \
	$(MBEDTLS_LIB_DIR)/blowfish.c \
	$(MBEDTLS_LIB_DIR)/camellia.c \
	$(MBEDTLS_LIB_DIR)/ccm.c \
	$(MBEDTLS_LIB_DIR)/chacha20.c \
	$(MBEDTLS_LIB_DIR)/chachapoly.c \
	$(MBEDTLS_LIB_DIR)/cipher.c \
	$(MBEDTLS_LIB_DIR)/cipher_wrap.c \
	$(MBEDTLS_LIB_DIR)/cmac.c \
	$(MBEDTLS_LIB_DIR)/ctr_drbg.c \
	$(MBEDTLS_LIB_DIR)/des.c \
	$(MBEDTLS_LIB_DIR)/dhm.c \
	$(MBEDTLS_LIB_DIR)/ecdh.c \
	$(MBEDTLS_LIB_DIR)/ecdsa.c \
	$(MBEDTLS_LIB_DIR)/ecjpake.c \
	$(MBEDTLS_LIB_DIR)/ecp.c \
	$(MBEDTLS_LIB_DIR)/ecp_curves.c \
	$(MBEDTLS_LIB_DIR)/entropy.c \
	$(MBEDTLS_LIB_DIR)/entropy_poll.c \
	$(MBEDTLS_LIB_DIR)/error.c \
	$(MBEDTLS_LIB_DIR)/gcm.c \
	$(MBEDTLS_LIB_DIR)/havege.c \
	$(MBEDTLS_LIB_DIR)/hkdf.c \
	$(MBEDTLS_LIB_DIR)/hmac_drbg.c \
	$(MBEDTLS_LIB_DIR)/md.c \
	$(MBEDTLS_LIB_DIR)/md2.c \
	$(MBEDTLS_LIB_DIR)/md4.c \
	$(MBEDTLS_LIB_DIR)/md5.c \
	$(MBEDTLS_LIB_DIR)/md_wrap.c \
	$(MBEDTLS_LIB_DIR)/memory_buffer_alloc.c \
	$(MBEDTLS_LIB_DIR)/nist_kw.c \
	$(MBEDTLS_LIB_DIR)/oid.c \
	$(MBEDTLS_LIB_DIR)/padlock.c \
	$(MBEDTLS_LIB_DIR)/pem.c \
	$(MBEDTLS_LIB_DIR)/pk.c \
	$(MBEDTLS_LIB_DIR)/pk_wrap.c \
	$(MBEDTLS_LIB_DIR)/pkcs12.c \
	$(MBEDTLS_LIB_DIR)/pkcs5.c \
	$(MBEDTLS_LIB_DIR)/pkparse.c \
	$(MBEDTLS_LIB_DIR)/pkwrite.c \
	$(MBEDTLS_LIB_DIR)/platform.c \
	$(MBEDTLS_LIB_DIR)/platform_util.c \
	$(MBEDTLS_LIB_DIR)/poly1305.c \
	$(MBEDTLS_LIB_DIR)/ripemd160.c \
	$(MBEDTLS_LIB_DIR)/rsa.c \
	$(MBEDTLS_LIB_DIR)/rsa_internal.c \
	$(MBEDTLS_LIB_DIR)/sha1.c \
	$(MBEDTLS_LIB_DIR)/sha256.c \
	$(MBEDTLS_LIB_DIR)/sha512.c \
	$(MBEDTLS_LIB_DIR)/threading.c \
	$(MBEDTLS_LIB_DIR)/timing.c \
	$(MBEDTLS_LIB_DIR)/version.c \
	$(MBEDTLS_LIB_DIR)/version_features.c \
	$(MBEDTLS_LIB_DIR)/xtea.c
MBEDTLS_X509_SRC = \
	$(MBEDTLS_LIB_DIR)/certs.c \
	$(MBEDTLS_LIB_DIR)/pkcs11.c \
	$(MBEDTLS_LIB_DIR)/x509.c \
	$(MBEDTLS_LIB_DIR)/x509_create.c \
	$(MBEDTLS_LIB_DIR)/x509_crl.c \
	$(MBEDTLS_LIB_DIR)/x509_crt.c \
	$(MBEDTLS_LIB_DIR)/x509_csr.c \
	$(MBEDTLS_LIB_DIR)/x509write_crt.c \
	$(MBEDTLS_LIB_DIR)/x509write_csr.c
MBEDTLS_TLS_SRC = \
	$(MBEDTLS_LIB_DIR)/debug.c \
	$(MBEDTLS_LIB_DIR)/net_sockets.c \
	$(MBEDTLS_LIB_DIR)/ssl_cache.c \
	$(MBEDTLS_LIB_DIR)/ssl_ciphersuites.c \
	$(MBEDTLS_LIB_DIR)/ssl_cli.c \
	$(MBEDTLS_LIB_DIR)/ssl_cookie.c \
	$(MBEDTLS_LIB_DIR)/ssl_srv.c \
	$(MBEDTLS_LIB_DIR)/ssl_ticket.c \
	$(MBEDTLS_LIB_DIR)/ssl_tls.c

# 所有源文件
ALL_SRC = $(XTRANS_SRC) $(XHTTPC_SRC) $(MBEDTLS_CRYPTO_SRC) $(MBEDTLS_X509_SRC) $(MBEDTLS_TLS_SRC)

# 构造.obj目录下的目标文件路径（按编译模式区分，避免冲突）
OBJS = \
	$(patsubst %.c, $(OBJ_DIR)/$(BUILD_TYPE)/%.o, $(XTRANS_SRC)) \
	$(patsubst %.c, $(OBJ_DIR)/$(BUILD_TYPE)/%.o, $(XHTTPC_SRC)) \
	$(patsubst $(MBEDTLS_LIB_DIR)/%.c, $(OBJ_DIR)/$(BUILD_TYPE)/$(MBEDTLS_LIB_DIR)/%.o, $(MBEDTLS_CRYPTO_SRC)) \
	$(patsubst $(MBEDTLS_LIB_DIR)/%.c, $(OBJ_DIR)/$(BUILD_TYPE)/$(MBEDTLS_LIB_DIR)/%.o, $(MBEDTLS_X509_SRC)) \
	$(patsubst $(MBEDTLS_LIB_DIR)/%.c, $(OBJ_DIR)/$(BUILD_TYPE)/$(MBEDTLS_LIB_DIR)/%.o, $(MBEDTLS_TLS_SRC))

# 默认目标（debug版）
all: $(TARGET)
	@echo "Build finished (DEBUG mode). Use 'make release' for optimized version."

# Release 目标（快速编译release版）
release:
	@$(MAKE) BUILD_TYPE=release  # 调用自身，指定BUILD_TYPE为release

# 链接目标：生成可执行文件后自动删除对应模式的.obj子目录
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "Build completed: $(TARGET) ($(BUILD_TYPE) mode)"
	@echo "Cleaning up intermediate files..."
	rm -rf $(OBJ_DIR)/$(BUILD_TYPE)  # 仅删除当前模式的中间文件，不影响其他模式

# 编译规则：自动创建.obj/[模式]目录及子目录
$(OBJ_DIR)/$(BUILD_TYPE)/%.o: %.c
	@mkdir -p $(dir $@)  # 递归创建带模式的目录（如.obj/release/mbedtls/library）
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理目标：删除所有.obj目录和可执行文件
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Clean completed (all build types)"

# 重新编译（支持指定模式，如 make rebuild BUILD_TYPE=release）
rebuild: clean $(TARGET)

# 帮助信息
help:
	@echo "Makefile for xtrans translation tool"
	@echo ""
	@echo "Usage:"
	@echo "  make                - Build DEBUG version (no optimization, with debug info)"
	@echo "  make release        - Build RELEASE version (max optimization, no debug info)"
	@echo "  make clean          - Remove all intermediate files and executable"
	@echo "  make rebuild        - Clean and rebuild current version (default DEBUG)"
	@echo "  make BUILD_TYPE=release - Build RELEASE version (alternative way)"
	@echo ""
	@echo "Release features:"
	@echo "  - Optimization: O3 (max speed/size optimization)"
	@echo "  - No debug info: DNDEBUG (disables assert())"
	@echo "  - Less warnings: Ignore unused functions/variables"

.PHONY: all release clean rebuild help