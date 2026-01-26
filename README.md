# xtrans - Lightweight Translation Tool

xtrans is a lightweight command-line translation tool that supports multiple translation engines and proxy configurations.

## Features

### Translation Engines
- **Google**: High-quality translation with auto-detect
- **Bing**: Fast and reliable results
- **MyMemory**: Free API with reasonable limits

### Proxy Support
- **SOCKS5**: `socks5://[user[:password]@]host:port`
- **SOCKS5h**: `socks5h://[user[:password]@]host:port` (remote DNS resolution)
- **HTTP CONNECT**: `http://[user[:password]@]host:port`

### Configuration Methods
- Command-line options: `-x/--proxy`
- Environment variables: `ALL_PROXY`, `HTTP_PROXY`, `HTTPS_PROXY`
- Explicit disable: `--no-proxy` or `-x none`

## Usage

### Basic Translation
```bash
# Auto-detect source language, translate to Chinese
./xtrans.exe "Hello world"

# Translate to French
./xtrans.exe -t fr "Hello world"

# Specify source and target languages
./xtrans.exe -s en -t fr "Hello world"

# Use specific translation engine
./xtrans.exe -e bing "Hello world"
```

### Proxy Configuration
```bash
# SOCKS5 proxy
./xtrans.exe -e google "Hello world" -t zh -x socks5://127.0.0.1:1080

# HTTP CONNECT proxy
./xtrans.exe -e mymemory "Hello world" -t fr --proxy http://127.0.0.1:8888

# Proxy with credentials
./xtrans.exe -e google "你好" -t en -x socks5://user:pass@proxy.example.com:1080

# SOCKS5h (remote DNS resolution)
./xtrans.exe -e google "Hello world" -t ja -x socks5h://127.0.0.1:1080
```

### Environment Variables
```bash
# Windows Command Prompt
set ALL_PROXY=socks5://127.0.0.1:1080
./xtrans.exe -e google "Hello world" -t zh

# PowerShell
$env:ALL_PROXY = "socks5://127.0.0.1:1080"
./xtrans.exe -e google "Hello world" -t zh
```

### Advanced Options
```bash
# Verbose output
./xtrans.exe -v "Hello world"

# List supported languages
./xtrans.exe -l

# Disable proxy
./xtrans.exe -e google "Hello world" -t zh --no-proxy
./xtrans.exe -e google "Hello world" -t zh -x none
```

## Compilation

### Prerequisites
- GCC with C99 support
- mbedTLS library (included in repository)

### Build
```bash
# Clean and build (Windows)
make clean && make

# Build with parallel compilation (faster)
make clean && make -j 4

# Build debug version
make clean && make debug
```

## Proxy Status

The proxy integration is complete and tested with:
- ✅ SOCKS5 proxies
- ✅ SOCKS5h proxies (remote DNS resolution)
- ✅ HTTP CONNECT proxies
- ✅ Proxy credentials support
- ✅ Command-line proxy configuration
- ✅ Environment variable proxy configuration
- ✅ No-proxy option

## Supported Languages

xtrans supports over 100 languages. Use `./xtrans.exe -l` to see the full list.

## Performance

Average response times (tested with local proxy):
- **Direct connection**: ~0.5 seconds per request
- **SOCKS5 proxy**: ~0.8 seconds per request
- **HTTP CONNECT proxy**: ~0.9 seconds per request

## Troubleshooting

### Proxy Connection Issues
1. Verify proxy server is running and accessible
2. Check proxy type matches URL (socks5:// vs http://)
3. Test connectivity with curl: `curl -x socks5h://127.0.0.1:1080 -v https://www.google.com`
4. Use verbose output to see errors: `./xtrans.exe -v "Hello world" -t zh -x socks5://127.0.0.1:1080`

### Authentication Errors
1. Check credentials format: `socks5://user:pass@host:port`
2. Verify password doesn't contain special characters that need encoding
3. Test credentials with curl: `curl -x socks5://user:pass@127.0.0.1:1080 -v https://www.google.com`

### Firewall Issues
- Check if firewall is blocking proxy connections
- Verify proxy server is configured to accept connections from your IP

## Project Structure

```
xtrans/
├── src/                # Source files
│   ├── xhttpc.c       # HTTP client with proxy support
│   ├── xhttpc.h       # HTTP client headers
│   ├── xtrans.c       # Main translation engine
│   ├── xtrans_google.c # Google Translate backend
│   ├── xtrans_bing.c   # Bing Translate backend
│   └── xtrans_mymemory.c # MyMemory backend
├── tests/              # Test files
├── docs/               # Documentation
├── PROXY_STATUS.md     # Proxy implementation status
├── PROXY_INTEGRATION_SUMMARY.md # Google Translate integration details
└── Makefile            # Build system
```

## License

MIT License - see LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Support

For issues or questions, please open an issue on the GitHub repository or contact the maintainer.
