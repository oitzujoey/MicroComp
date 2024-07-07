
#ifndef DUCKLIB_STRING_H
#define DUCKLIB_STRING_H

#include "core.h"

void DECLSPEC dl_string_isDigit(dl_bool_t *result, const char character);
void DECLSPEC dl_string_isHexadecimalDigit(dl_bool_t *result, const char character);
void DECLSPEC dl_string_isAlpha(dl_bool_t *result, const char character);
void DECLSPEC dl_string_isSpace(dl_bool_t *result, const char character);

dl_error_t DECLSPEC dl_string_toBool(dl_bool_t *result, const char *string, const dl_size_t string_length);
dl_error_t DECLSPEC dl_string_toPtrdiff(dl_ptrdiff_t *result, const char *string, const dl_size_t string_length);
dl_error_t DECLSPEC dl_string_toDouble(double *result, const char *string, const dl_size_t string_length);

void DECLSPEC dl_string_compare(dl_bool_t *result, const char *str1, const dl_size_t str1_length, const char *str2, const dl_size_t str2_length);
void DECLSPEC dl_string_compare_partial(dl_bool_t *result, const char *str1, const char *str2, const dl_size_t length);

#define dl_string_toLower(c) (((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c)
#define dl_string_toUpper(c) (((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : c)

#endif /* DUCKLIB_STRING_H */
