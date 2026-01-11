# Makefile for xtrans translation tool (Linux)
# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
INCLUDES = -I. -Imbedtls/include
# 库设置
LIBS = 
#LIBS = -liconv
# 目录设置
MBEDTLS_LIB_DIR = mbedtls/library
MBEDTLS_INC_DIR = mbedtls/include
# 中间文件目录
OBJ_DIR = .obj
# 目标可执行文件
TARGET = xtrans

# 源文件
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

# 构造.obj目录下的目标文件路径（保持与源文件对应的目录结构）
OBJS = \
	$(patsubst %.c, $(OBJ_DIR)/%.o, $(XTRANS_SRC)) \
	$(patsubst %.c, $(OBJ_DIR)/%.o, $(XHTTPC_SRC)) \
	$(patsubst $(MBEDTLS_LIB_DIR)/%.c, $(OBJ_DIR)/$(MBEDTLS_LIB_DIR)/%.o, $(MBEDTLS_CRYPTO_SRC)) \
	$(patsubst $(MBEDTLS_LIB_DIR)/%.c, $(OBJ_DIR)/$(MBEDTLS_LIB_DIR)/%.o, $(MBEDTLS_X509_SRC)) \
	$(patsubst $(MBEDTLS_LIB_DIR)/%.c, $(OBJ_DIR)/$(MBEDTLS_LIB_DIR)/%.o, $(MBEDTLS_TLS_SRC))

# 默认目标
all: $(TARGET)

# 链接目标：生成可执行文件后自动删除.obj目录
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "Build completed: $(TARGET)"
	@echo "Cleaning up intermediate files..."
	rm -rf $(OBJ_DIR)  # 编译成功后删除.obj目录

# 编译规则：自动创建.obj目录及子目录，将.o文件输出到对应路径
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)  # 递归创建目标文件所在目录（不存在时）
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理目标：删除可执行文件和.obj目录（支持手动清理）
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Clean completed"

# 重新编译
rebuild: clean all

# 帮助信息
help:
	@echo "Makefile for xtrans translation tool"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the xtrans executable (default, intermediate files in .obj/ and auto-deleted)"
	@echo "  clean   - Remove .obj/ directory and executable"
	@echo "  rebuild - Clean and rebuild"
	@echo "  help    - Show this help message"

.PHONY: all clean rebuild help
