#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "xhttpc.h"
#include "xtrans_google.h"
#define strndup(str, n) str?strncpy((char*)malloc(n + 1), str, n):NULL

// Google Translate result structure
typedef struct {
    char* translation;
    char* original;
    char* phonetics;
    char* detected_language;
    char* error;
    int success;
} google_result_t;

// Static cache for TK tokens
#define MAX_TK_CACHE 100
typedef struct {
    char* text;
    char* tk;
    time_t timestamp;
} tk_cache_entry_t;

static tk_cache_entry_t tk_cache[MAX_TK_CACHE];
static int tk_cache_size = 0;

// Forward declarations
static char* gen_tk(const char* text);
static char* build_google_url(const char* text, const char* source_lang,
                            const char* target_lang, const char* hl);
static google_result_t parse_google_response(const char* json_response);
static void clean_cache(void);
static char* url_encode_component(const char* str);

// XOR operation for TK generation
static int xor_32(int a, int b) {
    return a ^ b;
}

// Generate RL value for TK calculation
static int gen_rl(int a, const char* b_str) {
    // Parse string b_str to array of integers
    int b_values[100];  // Support up to 100 values
    int b_count = 0;

    const char* p = b_str;
    while (*p && b_count < 100) {
        if (*p == '[') {
            p++; // Skip opening bracket
            continue;
        }
        if (*p == ']' || *p == ',') {
            p++; // Skip separator or closing bracket
            continue;
        }
        if (isdigit(*p)) {
            b_values[b_count++] = atoi(p);
            while (*p && isdigit(*p)) p++;
        } else {
            p++;
        }
    }

    for (int i = 0; i < b_count; i++) {
        a = xor_32(a, b_values[i]);
    }

    return a;
}

// Generate TK token for Google Translate API
static char* gen_tk(const char* text) {
    if (!text) return NULL;

    // Check cache first
    for (int i = 0; i < tk_cache_size; i++) {
        if (tk_cache[i].text && strcmp(tk_cache[i].text, text) == 0 &&
            (time(NULL) - tk_cache[i].timestamp) < 3600) { // Cache for 1 hour
            return strndup(tk_cache[i].tk, strlen(tk_cache[i].tk));
        }
    }

    // Clean old cache entries if needed
    if (tk_cache_size >= MAX_TK_CACHE) {
        clean_cache();
    }

    // Current timestamp divided by 3600 (hours)
    time_t now = time(NULL);
    int tkk = (int)(now / 3600);

    // Fixed arrays from AWK implementation
    const char* ub_str = "[43,45,51,94,43,98,43,45,102]";
    const char* vb_str = "[43,45,97,94,43,54]";

    // Convert text to character codes
    int len = (int)strlen(text);
    int* d = malloc(len * sizeof(int));
    if (!d) return NULL;

    for (int i = 0; i < len; i++) {
        d[i] = (unsigned char)text[i];
    }

    int a = tkk;
    for (int e = 0; e < len; e++) {
        a = gen_rl(a + d[e], vb_str);
    }

    a = gen_rl(a, ub_str);

    // Handle negative numbers (32-bit signed)
    if (a < 0) {
        a = (a & 0x7fffffff) + 0x80000000;
    }

    a %= 1000000;

    // Create TK string
    char* tk = malloc(50);
    if (!tk) {
        free(d);
        return NULL;
    }

    snprintf(tk, 50, "%d.%d", a, a ^ tkk);

    // Cache result
    if (tk_cache_size < MAX_TK_CACHE) {
        tk_cache[tk_cache_size].text = strndup(text, strlen(text));
        tk_cache[tk_cache_size].tk = strndup(tk, strlen(tk));
        tk_cache[tk_cache_size].timestamp = now;
        tk_cache_size++;
    }

    free(d);
    return tk;
}

// Clean old cache entries
static void clean_cache(void) {
    time_t now = time(NULL);
    int new_size = 0;

    for (int i = 0; i < tk_cache_size; i++) {
        if ((now - tk_cache[i].timestamp) < 3600) { // Keep if less than 1 hour old
            if (new_size != i) {
                tk_cache[new_size] = tk_cache[i];
            }
            new_size++;
        } else {
            free(tk_cache[i].text);
            free(tk_cache[i].tk);
        }
    }

    tk_cache_size = new_size;
}

// URL encode component
static char* url_encode_component(const char* str) {
    if (!str) return NULL;

    size_t len = strlen(str);
    char* encoded = malloc(len * 3 + 1); // Worst case: all chars need encoding
    if (!encoded) return NULL;

    int j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' ||
            c == '.' || c == '~') {
            encoded[j++] = c;
        } else {
            sprintf(&encoded[j], "%%%02X", (unsigned char)c);
            j += 3;
        }
    }
    encoded[j] = '\0';

    return encoded;
}

// Build Google Translate API request URL
static char* build_google_url(const char* text, const char* source_lang,
                            const char* target_lang, const char* hl) {
    char* tk = gen_tk(text);
    if (!tk) return NULL;

    char* encoded_text = url_encode_component(text);
    if (!encoded_text) {
        free(tk);
        return NULL;
    }

    const char* qc = "qca"; // Use default quality check

    char* url = malloc(1024);
    if (!url) {
        free(tk);
        free(encoded_text);
        return NULL;
    }

    snprintf(url, 1024,
             "https://translate.googleapis.com/translate_a/single"
             "?client=gtx"
             "&ie=UTF-8&oe=UTF-8"
             "&dt=bd&dt=ex&dt=ld&dt=md&dt=rw&dt=rm&dt=ss&dt=t&dt=at&dt=gt"
             "&dt=%s"
             "&sl=%s&tl=%s&hl=%s"
             "&tk=%s"
             "&q=%s",
             qc, source_lang ? source_lang : "auto", target_lang, hl ? hl : "en",
             tk, encoded_text);

    free(tk);
    free(encoded_text);
    return url;
}

// Parse Google Translate JSON response
static google_result_t parse_google_response(const char* json_response) {
    google_result_t result = {0};

    if (!json_response || strlen(json_response) == 0) {
        result.error = strndup("Empty response", 10);
        return result;
    }

    // Check if response starts with '[' (JSON array)
    if (json_response[0] != '[') {
        result.error = strndup("Invalid response format", 20);
        return result;
    }

    // Extract main translations from data[0][0][0]
    char* translation = malloc(2048);
    if (!translation) {
        result.error = strndup("Memory allocation failed", 22);
        return result;
    }
    translation[0] = '\0';

    // Simple pattern matching approach
    // Look for pattern: [[["translation","original",...]
    const char* ptr = json_response;
    int found_translation = 0;

    // Find pattern [[[" (three brackets and opening quote)
    char* pattern_start = strstr(json_response, "[[[\"");
    if (!pattern_start) {
        result.error = strndup("Cannot find translation pattern", 28);
        free(translation);
        return result;
    }

    ptr = pattern_start + 4; // Skip "[[[\""

    // Extract translation until closing quote
    const char* text_start = ptr;
    while (*ptr && (*ptr != '"' || (ptr > text_start && *(ptr-1) == '\\'))) {
        ptr++;
    }

    if (*ptr == '"') {
        size_t len = ptr - text_start;
        if (len > 0 && len < 1000) { // Sanity check
            strncpy(translation, text_start, len);
            translation[len] = '\0';
            found_translation = 1;
        }
    }

    if (found_translation && strlen(translation) > 0) {
        result.translation = translation;
        result.success = 1;

        // Default language detection - assume source based on content or use "unknown"
        result.detected_language = strndup("unknown", 7);
    } else {
        free(translation);
        result.error = strndup("No translation found", 16);
    }

    return result;
}

// Main Google Translate function
static google_result_t translate_google_imp(const char* text, const char* source_lang,
                               const char* target_lang, int verbose, const char* proxy) {
    google_result_t result = {0};

    if (!text || strlen(text) == 0) {
        result.error = strndup("Empty text", 10);
        return result;
    }

    if (!target_lang) {
        result.error = strndup("Target language is required", 25);
        return result;
    }

    // Build request URL
    char* url = build_google_url(text, source_lang, target_lang, "en");
    if (!url) {
        result.error = strndup("Failed to build request URL", 26);
        return result;
    }

    if (verbose) {
        printf("[DEBUG] Request URL: %s\n", url);
    }

    // Configure HTTP client
    httpc_config_t config = {
        .server_host = "translate.googleapis.com",
        .server_port = "443",
        .is_https = 1,
        .ca_cert_path = "",
        .debug_level = verbose ? 1 : 0,

        .request = NULL,
        .method = "GET",
        .url_path = url + strlen("https://translate.googleapis.com"), // Extract path
        .content_type = NULL,
        .user_agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
        .data = NULL,
        .data_length = 0,
        .extra_headers = "Accept: */*\r\nAccept-Language: en-US,en;q=0.9",
        .proxy = proxy
    };

    // Initialize client and send request
    httpc_client_t* client = httpc_client_init(&config);
    if (!client) {
        result.error = strndup("Failed to initialize HTTP client", 30);
        free(url);
        return result;
    }

    char response[8192] = {0};
    httpc_response_t resp_info = {0};
    size_t actual_read = 0;

    httpc_err_t err = httpc_client_request(client, response, sizeof(response), &actual_read);
    httpc_client_free(client);
    free(url);

    if (err != HTTPC_SUCCESS) {
        result.error = malloc(100);
        snprintf(result.error, 100, "HTTP request failed: %d", err);
        return result;
    }

    // Parse response to extract content
    err = httpc_parse_response(response, &resp_info);
    const char* raw_content = resp_info.content_start ? resp_info.content_start : response;

    // Skip any content length prefix (like "192\n")
    const char* json_content = raw_content;
    if (isdigit(raw_content[0])) {
        while (*json_content && *json_content != '\n') {
            json_content++;
        }
        if (*json_content == '\n') {
            json_content++;
        }
    }

    // Parse response
    if (verbose) {
        printf("[DEBUG] Raw response: %.500s\n", json_content);
    }
    result = parse_google_response(json_content);

    if (verbose && result.success) {
        // Add original text to result
        result.original = strndup(text, strlen(text));
        printf("[DEBUG] Parsed translation: '%s'\n", result.translation);
    }

    return result;
}

// Free Google result structure
void free_google_result(google_result_t* result) {
    if (!result) return;

    free(result->translation);
    free(result->original);
    free(result->phonetics);
    free(result->detected_language);
    free(result->error);

    memset(result, 0, sizeof(google_result_t));
}

// Detect language using Google Translate
char* google_detect_language(const char* text) {
    google_result_t result = translate_google_imp(text, "auto", "en", 0, NULL);
    char* detected = NULL;

    if (result.success && result.detected_language) {
        detected = strndup(result.detected_language, strlen(result.detected_language));
    }

    free_google_result(&result);
    return detected;
}

// Initialize Google Translate system (call once at startup)
static inline void google_init(void) {
    memset(tk_cache, 0, sizeof(tk_cache));
    tk_cache_size = 0;
}

// Cleanup Google Translate system (call at shutdown)
static inline void google_cleanup(void) {
    for (int i = 0; i < tk_cache_size; i++) {
        free(tk_cache[i].text);
        free(tk_cache[i].tk);
    }
    tk_cache_size = 0;
}

char* translate_google(const char* text, const char* source, const char* target, int verbose, const char* proxy) {
    google_init();
    google_result_t result = translate_google_imp(text, source, target, verbose, proxy);
    char* translation = NULL;

    if (result.success && result.translation) {
        translation = strndup(result.translation, strlen(result.translation));
    } else if (result.error) {
        fprintf(stderr, "Google translation error: %s\n", result.error);
    }

    free_google_result(&result);
    google_cleanup();
    return translation;
}
