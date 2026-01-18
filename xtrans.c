#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "xhttpc.h"
#include "compat.h"
#include "xtrans_bing.h"

// Language codes mapping
typedef struct {
    const char* code;
    const char* name;
} lang_info_t;

static const lang_info_t LANGUAGE_NAMES[] = {
    {"af", "Afrikaans"}, {"sq", "Albanian"}, {"ar", "Arabic"}, {"hy", "Armenian"},
    {"az", "Azerbaijani"}, {"eu", "Basque"}, {"bn", "Bengali"}, {"bs", "Bosnian"},
    {"bg", "Bulgarian"}, {"ca", "Catalan"}, {"zh", "Chinese"}, {"zh-cn", "Chinese Simplified"},
    {"zh-tw", "Chinese Traditional"}, {"hr", "Croatian"}, {"cs", "Czech"}, {"da", "Danish"},
    {"nl", "Dutch"}, {"en", "English"}, {"eo", "Esperanto"}, {"et", "Estonian"},
    {"tl", "Filipino"}, {"fi", "Finnish"}, {"fr", "French"}, {"gl", "Galician"},
    {"ka", "Georgian"}, {"de", "German"}, {"el", "Greek"}, {"gu", "Gujarati"},
    {"ht", "Haitian Creole"}, {"ha", "Hausa"}, {"he", "Hebrew"}, {"hi", "Hindi"},
    {"hu", "Hungarian"}, {"is", "Icelandic"}, {"id", "Indonesian"}, {"ga", "Irish"},
    {"it", "Italian"}, {"ja", "Japanese"}, {"kn", "Kannada"}, {"kk", "Kazakh"},
    {"km", "Khmer"}, {"ko", "Korean"}, {"ku", "Kurdish"}, {"ky", "Kyrgyz"},
    {"lo", "Lao"}, {"la", "Latin"}, {"lv", "Latvian"}, {"lt", "Lithuanian"},
    {"lb", "Luxembourgish"}, {"mk", "Macedonian"}, {"mg", "Malagasy"}, {"ms", "Malay"},
    {"ml", "Malayalam"}, {"mt", "Maltese"}, {"mi", "Maori"}, {"mr", "Marathi"},
    {"mn", "Mongolian"}, {"my", "Myanmar"}, {"ne", "Nepali"}, {"no", "Norwegian"},
    {"ps", "Pashto"}, {"fa", "Persian"}, {"pl", "Polish"}, {"pt", "Portuguese"},
    {"pa", "Punjabi"}, {"ro", "Romanian"}, {"ru", "Russian"}, {"sm", "Samoan"},
    {"gd", "Scots Gaelic"}, {"sr", "Serbian"}, {"sn", "Shona"}, {"sd", "Sindhi"},
    {"si", "Sinhala"}, {"sk", "Slovak"}, {"sl", "Slovenian"}, {"so", "Somali"},
    {"es", "Spanish"}, {"su", "Sundanese"}, {"sw", "Swahili"}, {"sv", "Swedish"},
    {"tg", "Tajik"}, {"ta", "Tamil"}, {"te", "Telugu"}, {"th", "Thai"},
    {"tr", "Turkish"}, {"uk", "Ukrainian"}, {"ur", "Urdu"}, {"ug", "Uyghur"},
    {"uz", "Uzbek"}, {"vi", "Vietnamese"}, {"cy", "Welsh"}, {"xh", "Xhosa"},
    {"yi", "Yiddish"}, {"yo", "Yoruba"}, {"zu", "Zulu"},
    {NULL, NULL}
};

typedef struct {
    int verbose;
    int list_languages;
    char* text;
    char* source;
    char* target;
    char* engine;
    int use_bing;
} trans_config_t;

// MyMemory translation function
char* translate_mymemory(const char* text, const char* source, const char* target, int verbose) {
    // Convert text to UTF-8
    char utf8_buf[2048] = { 0 };
    int utf8_len = httpc_any_to_utf8(text, utf8_buf, sizeof(utf8_buf));
    if (utf8_len < 0) {
        fprintf(stderr, "Failed to convert text to UTF-8\n");
        return NULL;
    }

    // URL encode text
    char* encoded_text = httpc_url_encode(utf8_buf);
    if (!encoded_text) {
        fprintf(stderr, "Failed to encode text\n");
        return NULL;
    }

    // Build request dynamically
    char* request = malloc(2048);
    if (!request) {
        free(encoded_text);
        fprintf(stderr, "Failed to allocate memory for request\n");
        return NULL;
    }

    snprintf(request, 2048,
        "GET /get?q=%s&langpair=%s|%s HTTP/1.1\r\n"
        "Host: api.mymemory.translated.net\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n"
        "Connection: close\r\n"
        "Accept: application/json\r\n"
        "\r\n",
        encoded_text, source, target);

    free(encoded_text);

    if (verbose) {
        printf("HTTP Request:\n%s", request);
    }

    // Configure HTTP client
    httpc_config_t config = {
        .server_host = "api.mymemory.translated.net",
        .server_port = "443",
        .is_https = 1,
        .ca_cert_path = "",
        .request = request,
        .debug_level = 0
    };

    // Initialize client and send request
    httpc_client_t* client = httpc_client_init(&config);
    if (!client) {
        fprintf(stderr, "Failed to initialize HTTP client\n");
        return NULL;
    }

    // Receive response
    char response_buffer[512*1024];
    size_t actual_read = 0;

    httpc_err_t err = httpc_client_request(client, response_buffer, sizeof(response_buffer), &actual_read);
    httpc_client_free(client);

    if (err != HTTPC_SUCCESS) {
        fprintf(stderr, "HTTP request failed with error %d\n", err);
        return NULL;
    }

    response_buffer[actual_read] = '\0';

    if (verbose) {
        printf("HTTP Response:\n%s\n", response_buffer);
    }

    // Extract JSON body from HTTP response
    char* json_start = strstr(response_buffer, "\r\n\r\n");
    if (!json_start) {
        fprintf(stderr, "Invalid HTTP response format\n");
        return NULL;
    }
    json_start += 4;

    char buff[1024*10];
    if (!httpc_extract_pattern(json_start, "\"translatedText\":\"", "\"", buff, sizeof(buff))) {
        fprintf(stderr, "Failed to extract params_AbusePreventionHelper from response\n");
        return NULL;
    }

    char* result = malloc(strlen(buff) + 1);
    if (!result) {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }
    strcpy(result, buff);
    return result;
}

// Bing translation function (from xtrans.c)
int translate_bing(const char* text, const char* source_lang, const char* target_lang, char* result, size_t result_len, int verbose) {
    if (!text || !result || result_len == 0) return 0;

    // URL encode text
    char* encoded_text = httpc_url_encode(text);
    if (!encoded_text) {
        fprintf(stderr, "Failed to encode text\n");
        return 0;
    }

    // Build request - use proper setlang parameter based on language direction
    char request[1024];
    if (strcmp(source_lang, "zh") == 0 || strcmp(source_lang, "zh-cn") == 0) {
        // Chinese to English
        snprintf(request, sizeof(request),
            "GET /dict/search?q=%s&mkt=zh-CN&setlang=en HTTP/1.1\r\n"
            "Host: cn.bing.com\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n"
            "Connection: close\r\n"
            "\r\n",
            encoded_text);
    } else {
        // English to Chinese
        snprintf(request, sizeof(request),
            "GET /dict/search?q=%s&mkt=zh-CN&setlang=zh HTTP/1.1\r\n"
            "Host: cn.bing.com\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n"
            "Connection: close\r\n"
            "\r\n",
            encoded_text);
    }

    free(encoded_text);

    if (verbose) {
        printf("Bing Request:\n%s", request);
    }

    // Configure HTTP client
    httpc_config_t config = {
        .server_host = "cn.bing.com",
        .server_port = "443",
        .is_https = 1,
        .ca_cert_path = "",
        .request = request,
        .debug_level = verbose ? 1 : 0
    };

    // Initialize client and send request
    httpc_client_t* client = httpc_client_init(&config);
    if (!client) {
        fprintf(stderr, "Failed to initialize HTTP client\n");
        return 0;
    }

    // Receive response
    char response_buffer[16384];
    size_t actual_read = 0;

    httpc_err_t err = httpc_client_request(client, response_buffer, sizeof(response_buffer), &actual_read);
    httpc_client_free(client);

    if (err != HTTPC_SUCCESS) {
        if (verbose) {
            fprintf(stderr, "Bing request failed with error %d\n", err);
        }
        return 0;
    }

    if (verbose) {
        printf("Bing Response:\n%s\n", response_buffer);
    }

    // Parse Bing response
    const char* meta_start = strstr(response_buffer, "<meta name=\"description\" content=\"");
    if (!meta_start) {
        if (verbose) {
            fprintf(stderr, "Bing parse failed: no meta description found\n");
        }
        return 0;
    }

    meta_start += strlen("<meta name=\"description\" content=\"");

    const char* web_end = strstr(meta_start, "\" />");
    if (!web_end) {
        web_end = response_buffer + strlen(response_buffer);
    }

    size_t web_len = web_end - meta_start;
    size_t copy_len = (web_len > result_len - 1) ? result_len - 1 : web_len;
    strncpy(result, meta_start, copy_len);
    result[copy_len] = '\0';

    if (verbose) {
        printf("Extracted Bing result: %s\n", result);
    }

    return 1;
}

// Helper function to check if Bing result indicates failed translation
int is_bing_translation_failed(const char* result) {
    if (!result || strlen(result) == 0) return 1;

    // Check for failed/placeholder responses
    if (strstr(result, "search?q=") != NULL) return 1;

    // Check for generic dictionary responses (no actual translation)
    if (strcmp(result, "Dictionary") == 0) return 1;
    if (strcmp(result, "词典") == 0) return 1;

    // Check if result just repeats the input or is too short
    if (strlen(result) < 3) return 1;

    return 0;
}

// Hybrid translation function with engine tracking
char* translate_hybrid_with_engine(const char* text, const char* source_lang, const char* target_lang, int verbose, const char** engine_used) {
    *engine_used = "MyMemory";  // Default to MyMemory

    char utf8_buf[512] = { 0 };
    int utf8_len = httpc_any_to_utf8(text, utf8_buf, sizeof(utf8_buf));
    if (utf8_len < 0) {
        fprintf(stderr, u8"编码转换失败\n");
        return NULL;
    }

    if (verbose) {
        printf("Trying Bing translation first (text length: %zu)\n", strlen(text));
    }

    // Always try Bing first, then evaluate the result
    char* bing_result = malloc(1024);
    if (!bing_result) {
        fprintf(stderr, "Failed to allocate memory for Bing result\n");
        return NULL;
    }

    char* result = NULL;

    if (translate_bing(utf8_buf, source_lang, target_lang, bing_result, 1024, verbose)) {
        if (verbose) {
            printf("Bing request successful, result: '%s'\n", bing_result);
        }

        // Evaluate Bing result quality
        if (is_bing_translation_failed(bing_result)) {
            if (verbose) {
                printf("Bing result indicates failed translation, falling back to MyMemory\n");
            }
            free(bing_result);
            result = translate_mymemory(utf8_buf, source_lang, target_lang, verbose);
        } else {
            if (verbose) {
                printf("Bing translation is valid, using Bing result\n");
            }
            *engine_used = "Bing";
            result = bing_result;
        }
    } else {
        if (verbose) {
            printf("Bing request failed, falling back to MyMemory\n");
        }
        result = translate_mymemory(utf8_buf, source_lang, target_lang, verbose);
        if(result) free(bing_result);
    }
    if (!result) {
        if (verbose) {
            printf("MyMemory request failed, falling back to Bing\n");
        }

        if (translate_bing_long(utf8_buf, source_lang, target_lang, bing_result, 1024, verbose) > 0) {
            *engine_used = "MyMemory";
            result = bing_result;
        }
    }

    return result;
}

// List supported languages
void list_languages() {
    printf("Supported languages:\n");
    for (int i = 0; LANGUAGE_NAMES[i].code; i++) {
        printf("  %-8s - %s\n", LANGUAGE_NAMES[i].code, LANGUAGE_NAMES[i].name);
    }
}

// Print usage
void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] TEXT\n\n", program_name);
    printf("Translation tool using hybrid approach (Bing + MyMemory) with mbedtls HTTPS support\n\n");
    printf("Options:\n");
    printf("  -s, --source LANG    Source language (default: auto)\n");
    printf("  -t, --target LANG    Target language (default: auto-detect)\n");
    printf("  -e, --engine ENGINE   Translation engine (default: hybrid)\n");
    printf("  -l, --list           List supported languages\n");
    printf("  -v, --verbose        Verbose output\n");
    printf("  -h, --help           Show this help message\n");
    printf("  --no-bing           Disable Bing translation, use MyMemory only\n");
    printf("\n");
    printf("Engines:\n");
    printf("  hybrid (default) - Try Bing for short sentences, fallback to MyMemory\n");
    printf("  mymemory        - Use MyMemory only\n");
    printf("  bing            - Use Bing only\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s \"Hello world\"           # Auto-detect, translate to Chinese\n", program_name);
    printf("  %s \"Hello\" -t zh-cn        # English to Chinese\n", program_name);
    printf("  %s \"你好\" -t en             # Chinese to English\n", program_name);
    printf("  %s --engine bing 你好      # Force Bing translation\n", program_name);
    printf("  %s -e mymemory Hello       # Force MyMemory translation\n", program_name);
    printf("  %s --list                  # Show supported languages\n", program_name);
    printf("\n");
    printf("Optimization: Short sentences (<100 chars) use Bing for fast translation,\n");
    printf("Long sentences use MyMemory. Enable --verbose to see which service is used.\n");
}

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    BOOL output_utf8 = SetConsoleOutputCP(CP_UTF8);
    BOOL input_utf8 = SetConsoleCP(CP_UTF8);
    if (!output_utf8 || !input_utf8) {
        fprintf(stderr, "Warning：Failed to set console output to UTF-8; Chinese text may appear garbled.\n");
    }
    console_set_font_to_consolas();
    #endif

    trans_config_t config = {0};
    config.engine = "hybrid";

    // Parse command line arguments
    static struct option long_options[] = {
        {"source", required_argument, 0, 's'},
        {"target", required_argument, 0, 't'},
        {"engine", required_argument, 0, 'e'},
        {"list", no_argument, 0, 'l'},
        {"verbose", no_argument, 0, 'v'},
        {"no-bing", no_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "s:t:e:lvh", long_options, NULL)) != -1) {
        switch (c) {
            case 's':
                config.source = optarg;
                break;
            case 't':
                config.target = optarg;
                break;
            case 'e':
                config.engine = optarg;
                break;
            case 'l':
                config.list_languages = 1;
                break;
            case 'v':
                config.verbose = 1;
                break;
            case 0:
                config.use_bing = 0; // --no-bing
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
            default:
                break;
        }
    }

    if (config.list_languages) {
        list_languages();
        return 0;
    }

    // Get text to translate
    if (optind >= argc) {
        fprintf(stderr, "Error: Text to translate is required\n");
        print_usage(argv[0]);
        return 1;
    }
    config.text = argv[optind];

    // Determine source and target languages
    const char* source_lang = config.source;
    const char* target_lang = config.target;

    // Auto-detect if target not specified
    if (!target_lang) {
        const char* detected = httpc_detect_language(config.text);
        if (strcmp(detected, "zh-cn") == 0) {
            target_lang = "en";
            source_lang = "zh";
        } else {
            target_lang = "zh-cn";
            source_lang = "en";
        }
        if (config.verbose) {
            printf("Auto-detect: %s, using %s -> %s\n", detected, source_lang, target_lang);
        }
    } else if (strcmp(config.source, "auto") == 0) {
        // Auto-detect source if target is specified
        const char* detected = httpc_detect_language(config.text);
        if (strcmp(detected, target_lang) != 0) {
            source_lang = detected;
            if (config.verbose) {
                printf("Auto-detect source: %s\n", source_lang);
            }
        }
    }

    // Determine engine
    if (strcmp(config.engine, "mymemory") == 0) {
        config.use_bing = 0;
    } else if (strcmp(config.engine, "bing") == 0) {
        config.use_bing = 1;
    } else {
        config.use_bing = 1; // Default to bing for hybrid
    }

    // Translate
    char* result = NULL;
    const char* engine_used = "unknown";

    if (strcmp(config.engine, "mymemory") == 0) {
        engine_used = "MyMemory";
        result = translate_mymemory(config.text, source_lang, target_lang, config.verbose);
    } else if (strcmp(config.engine, "bing") == 0) {
        engine_used = "Bing";
        char* bing_result = malloc(1024);
        if (bing_result && translate_bing_long(config.text, source_lang, target_lang, bing_result, 1024, config.verbose)) {
            result = bing_result;
        } else {
            if (bing_result) free(bing_result);
            result = NULL;
        }
    } else {
        // For hybrid mode, determine which engine was actually used
        result = translate_hybrid_with_engine(config.text, source_lang, target_lang, config.verbose, &engine_used);
    }

    if (result) {
        printf("[%s] %s\n", engine_used, result);
        free(result);
        return 0;
    } else {
        fprintf(stderr, "Translation failed\n");
        return 1;
    }
}
