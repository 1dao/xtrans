#ifndef BING_LONG_H
#define BING_LONG_H

#include <stddef.h>

// Bing long sentence translation function based on bing_trans.py
// Implements 4-step process from Python reference implementation
int translate_bing_long(const char* text, const char* source_lang, const char* target_lang, char* result, size_t result_len, int verbose);

#endif // BING_LONG_H
