#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "xargs.h"
#include "xhttpc.h"
#include "xtrans_bing.h"
#include "xtrans_google.h"

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

// MyMemory translation function
char* translate_mymemory(const char* text, const char* source, const char* target, int verbose, const char* proxy) {
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

    char url[4096];
    snprintf(url, sizeof(url),
             "/get?q=%s&langpair=%s|%s",
             encoded_text, source, target);
    free(encoded_text);

    // Configure HTTP client
    httpc_config_t config = {
        .server_host = "api.mymemory.translated.net",
        .server_port = "443",
        .is_https = 1,
        .ca_cert_path = "",
        .debug_level = verbose ? 1 : 0,

        // HTTP request (complete string)
        .request = NULL,

        // HTTP request components (not used when request is provided)
        .method = "GET",
        .url_path = url,
        .content_type = NULL,
        .user_agent = NULL,
        .data = NULL,
        .data_length = 0,
        .extra_headers = "Accept: application/json",

        // Proxy configuration
        .proxy = proxy
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
        printf("[DEBUG] HTTP Response Data:\n%s\n", response_buffer);
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
// Returns: >0 on success, 0 on failure, -1 on unsupported language pair
int translate_bing(const char* text, const char* source_lang, const char* target_lang, char* result, size_t result_len, int verbose, const char* proxy) {
    if (!text || !result || result_len == 0) return 0;

    if (!source_lang || !target_lang) {
        fprintf(stderr, "[DEBUG] translate_bing: source_lang or target_lang is NULL\n");
        return 0;
    }

    // Check if Bing supports this language pair (Bing dict only supports Chinese and English)
    int is_zh_source = (strcmp(source_lang, "zh") == 0 || strcmp(source_lang, "zh-cn") == 0 || strcmp(source_lang, "zh-tw") == 0);
    int is_zh_target = (strcmp(target_lang, "zh") == 0 || strcmp(target_lang, "zh-cn") == 0 || strcmp(target_lang, "zh-tw") == 0);
    int is_en_source = (strcmp(source_lang, "en") == 0);
    int is_en_target = (strcmp(target_lang, "en") == 0);

    // Bing dict only supports Chinese <-> English
    int is_supported = ((is_zh_source && is_en_target) || (is_en_source && is_zh_target));

    if (!is_supported) {
        return -1;  // Signal unsupported language pair
    }

    // URL encode text
    char* encoded_text = httpc_url_encode(text);
    if (!encoded_text) {
        fprintf(stderr, "Failed to encode text\n");
        return 0;
    }

    // Build request - use proper setlang parameter based on language direction
    char url[2048];
    if (is_zh_source && is_en_target) {
        // Chinese to English
        snprintf(url, sizeof(url), "/dict/search?q=%s&mkt=zh-CN&setlang=en", encoded_text);
    } else {
        // English to Chinese
        snprintf(url, sizeof(url), "/dict/search?q=%s&mkt=zh-CN&setlang=zh", encoded_text);
    }
    free(encoded_text);

    // Configure HTTP client
    httpc_config_t config = {
        .server_host = "cn.bing.com",
        .server_port = "443",
        .is_https = 1,
        .ca_cert_path = "",
        .debug_level = verbose ? 1 : 0,

        // HTTP request (complete string)
        .request = NULL,

        // HTTP request components (not used when request is provided)
        .method = "GET",
        .url_path = url,
        .content_type = NULL,
        .user_agent = NULL,
        .data = NULL,
        .data_length = 0,
        .extra_headers = NULL,

        // Proxy configuration
        .proxy = proxy
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
char* translate_hybrid_with_engine(const char* text, const char* source_lang, const char* target_lang, int verbose, const char** engine_used, const char* proxy) {
    *engine_used = "Bing";  // Default to Bing Long

    char utf8_buf[512] = { 0 };
    int utf8_len = httpc_any_to_utf8(text, utf8_buf, sizeof(utf8_buf));
    if (utf8_len < 0) {
        fprintf(stderr, "Encode inpute failed\n");
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

    int bing_status = translate_bing(utf8_buf, source_lang, target_lang, bing_result, 1024, verbose, proxy);
    if (bing_status > 0) {
        if (verbose) {
            printf("[DEBUG] Bing request successful, result: '%s'\n", bing_result);
        }

        // Evaluate Bing result quality
        if (is_bing_translation_failed(bing_result)) {
            if (verbose) {
                printf("[DEBUG] Bing result indicates failed translation, falling back to MyMemory\n");
            }
            free(bing_result);
            char* fallback_result = malloc(1024);
            if (fallback_result) {
                if (translate_bing_long(utf8_buf, source_lang, target_lang, fallback_result, 1024, verbose, proxy) > 0) {
                    *engine_used = "Bing Long";
                    result = fallback_result;
                } else {
                    free(fallback_result);
                    result = NULL;
                }
            }
        } else {
            if (verbose) {
                printf("[DEBUG] Bing translation is valid, using Bing result\n");
            }
            *engine_used = "Bing";
            result = bing_result;
        }
    } else if (bing_status == -1) {
        if (verbose) {
            printf("[DEBUG] Bing doesn't support %s->%s, using MyMemory\n", source_lang, target_lang);
        }
        free(bing_result);
        char* fallback_result = malloc(1024);
        if (fallback_result) {
            if (translate_bing_long(utf8_buf, source_lang, target_lang, fallback_result, 1024, verbose, proxy) > 0) {
                *engine_used = "Bing Long";
                result = fallback_result;
            } else {
                free(fallback_result);
                result = NULL;
            }
        }
    } else {
        if (verbose) {
            printf("[DEBUG] Bing request failed, falling back to Bing Long\n");
        }
        free(bing_result);
        char* fallback_result = malloc(1024);
        if (fallback_result) {
            if (translate_bing_long(utf8_buf, source_lang, target_lang, fallback_result, 1024, verbose, proxy) > 0) {
                *engine_used = "Bing Long";
                result = fallback_result;
            } else {
                free(fallback_result);
                result = NULL;
            }
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
    printf("  -x, --proxy URL      Proxy server URL (e.g., socks5://127.0.0.1:1080 or http://127.0.0.1:8888)\n");
    printf("  --no-bing           Disable Bing translation, use MyMemory only\n");
    printf("\n");
    printf("Engines:\n");
    printf("  hybrid (default) - Try Bing for short sentences, fallback to MyMemory\n");
    printf("  mymemory        - Use MyMemory only\n");
    printf("  bing            - Use Bing only\n");
    printf("  google          - Use Google only\n");
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
    // init console
    console_set_consolas_font();

    // Parse command line arguments using xargs
    xArgsCFG configs[] = {
        {'s', "source", NULL, 0},
        {'t', "target", NULL, 0},
        {'e', "engine", "hybrid", 0},
        {'l', "list", NULL, 1},
        {'v', "verbose", NULL, 1},
        {'h', "help", NULL, 1},
        {'x', "proxy", NULL, 0},
        {0, "no-proxy", NULL, 1},
        {0, "no-bing", NULL, 1}
    };
    xargs_init(configs, sizeof(configs)/sizeof(configs[0]), argc, argv);

    const char* source_lang = xargs_get("s");
    const char* target_lang = xargs_get("t");
    const char* engine      = xargs_get("e");
    const char* list_lang   = xargs_get("l");
    const char* verbose     = xargs_get("v");
    const char* help_val    = xargs_get("h");
    const char* no_proxy    = xargs_get("no-proxy");
    const char* proxy_val   = NULL;
    if(!no_proxy) {
        proxy_val = xargs_get("x");
        if(!proxy_val)
            proxy_val = xargs_get("HTTP_PROXY");
        if(!proxy_val)
            proxy_val = xargs_get("http_proxy");
        if(!proxy_val)
            proxy_val = xargs_get("HTTPS_PROXY");
        if(!proxy_val)
            proxy_val = xargs_get("https_proxy");
        if(!proxy_val)
            proxy_val = xargs_get("ALL_PROXY");
        if(!proxy_val)
            proxy_val = xargs_get("all_proxy");
    }

    if (help_val) {
        print_usage(argv[0]);
        fflush(stdout);
        xargs_cleanup();
        return 0;
    }

    if (list_lang) {
        list_languages();
        xargs_cleanup();
        return 0;
    }

    // Get text to translate
    const char* text = xargs_get_other();
    if (!text || !text[0]) {
        fprintf(stderr, "Error: Text to translate is required\n");
        print_usage(argv[0]);
        xargs_cleanup();
        return 1;
    }

    // Auto-detect source and target languages if target not specified
    if (!target_lang) {
        const char* detected = httpc_detect_language(text);
        if (strcmp(detected, "zh-cn") == 0) {
            target_lang = "en";
            source_lang = "zh";
        } else {
            target_lang = "zh-cn";
            source_lang = "en";
        }
        if (verbose) {
            printf("[DEBUG] Auto-detect: %s, using %s -> %s\n", detected, source_lang, target_lang);
        }
    } else if (!source_lang || (source_lang && strcmp(source_lang, "auto") == 0)) {
        // Auto-detect source if target is specified but source is not
        const char* detected = httpc_detect_language(text);
        if (strcmp(detected, target_lang) != 0) {
            source_lang = detected;
            if (verbose) {
                printf("Auto-detect source: %s\n", source_lang);
            }
        }
    }
    if(verbose)
        printf("[DEBUG] proxy: %s\n", proxy_val);

    // Translate
    char* result = NULL;
    const char* engine_used = "unknown";
    if (strcmp(engine, "mymemory") == 0) {
        engine_used = "MyMemory";
        result = translate_mymemory(text, source_lang, target_lang, verbose?1:0, proxy_val);
    } else if (strcmp(engine, "bing") == 0) {
        engine_used = "Bing";
        char* bing_result = malloc(1024);
        if (bing_result && translate_bing_long(text, source_lang, target_lang, bing_result, 1024, verbose?1:0, proxy_val)) {
            result = bing_result;
        } else {
            if (bing_result) free(bing_result);
            result = NULL;
        }
    } else if (strcmp(engine, "google") == 0) {
        engine_used = "Google";
        result = translate_google(text, source_lang, target_lang, verbose?1:0, proxy_val);
    } else if (strcmp(engine, "deepl") == 0) {
        // engine_used = "DeepL";
        // char* deepl_result = malloc(1024);
        // if (deepl_result && translate_deepl(config.text, source_lang, target_lang, deepl_result, 1024, config.verbose, config.proxy)) {
        //     result = deepl_result;
        // } else {
        //     if (deepl_result) free(deepl_result);
        //     result = NULL;
        // }
    } else if (strcmp(engine, "yandex") == 0) {
        // engine_used = "Yandex";
        // char* yandex_result = malloc(1024);
        // if (yandex_result && translate_yandex(config.text, source_lang, target_lang, yandex_result, 1024, config.verbose, config.proxy)) {
        //     result = yandex_result;
        // } else {
        //     if (yandex_result) free(yandex_result);
        //     result = NULL;
        // }
    } else {
        // For hybrid mode, determine which engine was actually used
        result = translate_hybrid_with_engine(text, source_lang, target_lang, verbose?1:0, &engine_used, proxy_val);
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
