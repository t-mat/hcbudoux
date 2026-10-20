// hcbudoux.h
// ==========
//
// Header-only C11 implementation of BudouX line break library for Chinese, Japanese, and Thai languages.
//
//
// Usage
// -----
//
// Follow stb-style header only library convention.  Therefore, you should do this:
//
//     ```C
//     #define HCBUDOUX_IMPLEMENTATION
//     #include "hcbudoux.h"
//     ```
//
// in *one* C or C++ file to create the implementation.
//
//
// Example
// -------
//
// gcc:  gcc -std=c11 -I include example.c
// MSVC: cl.exe /utf-8 /nologo -I include example.c
//
//     ```C
//     // example.c
//     #define HCBUDOUX_IMPLEMENTATION
//     #include "hcbudoux.h"
//     #include <assert.h> // static_assert
//     #include <stdio.h>  // printf
//     #include <string.h> // strlen
//     #if defined(_WIN32)
//     #    define WIN32_LEAN_AND_MEAN
//     #    include <Windows.h>
//     #endif
//
//     #if defined(_WIN32)
//     void init(void) { SetConsoleOutputCP(CP_UTF8); }
//     #else
//     void init(void) {}
//     #endif
//
//     int main(int argc, const char **argv) {
//         static const char utf8Str[]  = u8"次の決闘がまもなく始まる！";
//         const int         utf8Strlen = (int)strlen(utf8Str);
//
//         (void)argc;
//         (void)argv;
//
//         static_assert(sizeof(utf8Str) == 40, "Basic UTF-8 test");
//         init();
//
//         hcbudoux_ctx ctx;
//         hcbudoux_init(&ctx, utf8Str, utf8Strlen);
//
//         hcbudoux_span span;
//         while (hcbudoux_getnext_ja(&ctx, &span)) {
//             const char *p = utf8Str + span.offset;
//             int         n = span.length;
//             printf("'%.*s'\n", n, p);
//         }
//         // We don't need to "close" hcbudoux_ctx.
//     }
//     ```
//
// Output:
// '次の'
// '決闘が'
// 'まもなく'
// '始まる！'
//
//
// License
// -------
//
// SPDX-License-Identifier: CC0-1.0 AND Apache-2.0
//
// "hcbudoux.h" by Takayuki Matsuoka.
// To the extent possible under law, the person who associated CC0-1.0 with "hcbudoux.h"
// has waived all copyright and related or neighboring rights to "hcbudoux.h".
// See https://creativecommons.org/publicdomain/zero/1.0/ for CC0-1.0 legalcode.
//
// The model tables ("BudouX Models" section) are generated from the models of BudouX
// (https://github.com/google/budoux), licensed under the Apache License, Version 2.0.
// The full license text is at the end of this file.

#ifndef HCBUDOUX_H_INCLUDED
#define HCBUDOUX_H_INCLUDED 1

#ifndef HCBUDOUX_DONT_INCLUDE_STD
#include <stdbool.h>  // bool, true, false
#include <stdint.h>   // uint8_t, uint32_t, uint64_t
#endif

// [[nodiscard]] is C++17.  Expands to nothing in C and in older C++.
// The language version is checked in addition to __has_cpp_attribute because clang reports the
// attribute as available in C++11/14 mode and then warns under -Wpedantic.  MSVC reports the
// version in _MSVC_LANG unless /Zc:__cplusplus is given.
#if defined(__cplusplus) && ((__cplusplus >= 201703L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)))
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define HCBUDOUX_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#if !defined(HCBUDOUX_NODISCARD)
#define HCBUDOUX_NODISCARD
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ======================
//   hcbudoux Public API
// vvvvvvvvvvvvvvvvvvvvvv
//
enum {
  hcbudoux_version_major = 0,
  hcbudoux_version_minor = 10,
  hcbudoux_version_patch = 1,
};

typedef struct hcbudoux_ctx {
  struct {
    const uint8_t *utf8_str;
    int32_t utf8_str_size_in_bytes;
    int32_t curr_index;
    int32_t last_index;
    uint32_t utf32s[6];
    int32_t indices[6];
  } impl;
} hcbudoux_ctx;

typedef struct hcbudoux_span {
  int32_t offset;  // public member: Offset in bytes from the beginning of utf8_str.
  int32_t length;  // public member: Length in bytes.
} hcbudoux_span;

// Initialize a parser context with a UTF-8 string.
// utf8_str is encoded in UTF-8.  utf8_str must outlive hcbudoux_ctx.
// We don't need to "close" hcbudoux_ctx since it doesn't allocate dynamic resources.
// hcbudoux doesn't require that utf8_str is terminated with '\0'.
void hcbudoux_init(hcbudoux_ctx *ctx, const void *utf8_str, int32_t utf8_str_size_in_bytes);

// Get the next string view of the specific language.
// Returns false when the parser reaches the end of utf8_str.
// When it returns true, span contains a valid offset and length of the string view.
//
// The string view is not terminated with `\0'.
// Use span->length to terminate the string view properly.
HCBUDOUX_NODISCARD bool hcbudoux_getnext_ja(hcbudoux_ctx *ctx, hcbudoux_span *span);
HCBUDOUX_NODISCARD bool hcbudoux_getnext_ja_knbc(hcbudoux_ctx *ctx, hcbudoux_span *span);
HCBUDOUX_NODISCARD bool hcbudoux_getnext_th(hcbudoux_ctx *ctx, hcbudoux_span *span);
HCBUDOUX_NODISCARD bool hcbudoux_getnext_zh_hans(hcbudoux_ctx *ctx, hcbudoux_span *span);
HCBUDOUX_NODISCARD bool hcbudoux_getnext_zh_hant(hcbudoux_ctx *ctx, hcbudoux_span *span);

//
// ^^^^^^^^^^^^^^^^^^^^^^
//   hcbudoux Public API
// ======================

//
// Implementation
//
#if defined(HCBUDOUX_IMPLEMENTATION)

#ifndef HCBUDOUX_IMPL_TEMPLATE
#define HCBUDOUX_IMPL_TEMPLATE(...)
#endif

#if !defined(HCBUDOUX_USE_JA) && !defined(HCBUDOUX_USE_JA_KNBC) && !defined(HCBUDOUX_USE_TH) && \
    !defined(HCBUDOUX_USE_ZH_HANS) && !defined(HCBUDOUX_USE_ZH_HANT)
#define HCBUDOUX_USE_JA 1
#define HCBUDOUX_USE_JA_KNBC 1
#define HCBUDOUX_USE_TH 1
#define HCBUDOUX_USE_ZH_HANS 1
#define HCBUDOUX_USE_ZH_HANT 1
#endif

//
// Public API : Initialize
//
void hcbudoux_init(hcbudoux_ctx *ctx, const void *utf8_str, int32_t utf8_str_size_in_bytes) {
  ctx->impl.utf8_str = (const uint8_t *)utf8_str;
  ctx->impl.utf8_str_size_in_bytes = utf8_str_size_in_bytes < 0 ? 0 : utf8_str_size_in_bytes;
  ctx->impl.curr_index = 0;
  ctx->impl.last_index = 0;
  for (int i = 0, n = (int)(sizeof(ctx->impl.utf32s) / sizeof(ctx->impl.utf32s[0])); i < n; ++i) {
    ctx->impl.utf32s[i] = 0;
  }
  for (int i = 0, n = (int)(sizeof(ctx->impl.indices) / sizeof(ctx->impl.indices[0])); i < n; ++i) {
    ctx->impl.indices[i] = -1;  // -1 : slot not filled yet
  }
}

//
// Utility functions
//
typedef enum hcbudoux_impl_lang {
  hcbudoux_impl_lang_ja,
  hcbudoux_impl_lang_ja_knbc,
  hcbudoux_impl_lang_th,
  hcbudoux_impl_lang_zh_hans,
  hcbudoux_impl_lang_zh_hant,
} hcbudoux_impl_lang;

typedef struct hcbudoux_impl_item1 {
  uint32_t var;
  int score;
} hcbudoux_impl_item1;

typedef struct hcbudoux_impl_item2 {
  uint64_t var;
  int score;
} hcbudoux_impl_item2;

typedef struct hcbudoux_impl_item3 {
  uint64_t var;
  int score;
} hcbudoux_impl_item3;

static int hcbudoux_impl_find1(const hcbudoux_impl_item1 *base, int len, uint32_t x) {
  if (len <= 0) {
    return 0;
  }
  while (len > 1) {
    const int half = len / 2;
    base += (base[half - 1].var < x) * half;
    len -= half;
  }
  return base->var == x ? base->score : 0;
}

static int hcbudoux_impl_find2(const hcbudoux_impl_item2 *base, int len, uint32_t x0, uint32_t x1) {
  if (len <= 0) {
    return 0;
  }
  const uint64_t x = ((uint64_t)x1) | (((uint64_t)x0) << 21);
  while (len > 1) {
    const int half = len / 2;
    base += (base[half - 1].var < x) * half;
    len -= half;
  }
  return base->var == x ? base->score : 0;
}

static int hcbudoux_impl_find3(const hcbudoux_impl_item3 *base, int len, uint32_t x0, uint32_t x1, uint32_t x2) {
  if (len <= 0) {
    return 0;
  }
  const uint64_t x = ((uint64_t)x2) | (((uint64_t)x1) << 21) | (((uint64_t)x0) << 42);
  while (len > 1) {
    const int half = len / 2;
    base += (base[half - 1].var < x) * half;
    len -= half;
  }
  return base->var == x ? base->score : 0;
}

static int hcbudoux_impl_compute_score_from_tables(
    int base_score, const hcbudoux_impl_item1 *uw1, int uw1_count, const hcbudoux_impl_item1 *uw2, int uw2_count,
    const hcbudoux_impl_item1 *uw3, int uw3_count, const hcbudoux_impl_item1 *uw4, int uw4_count,
    const hcbudoux_impl_item1 *uw5, int uw5_count, const hcbudoux_impl_item1 *uw6, int uw6_count,
    const hcbudoux_impl_item2 *bw1, int bw1_count, const hcbudoux_impl_item2 *bw2, int bw2_count,
    const hcbudoux_impl_item2 *bw3, int bw3_count, const hcbudoux_impl_item3 *tw1, int tw1_count,
    const hcbudoux_impl_item3 *tw2, int tw2_count, const hcbudoux_impl_item3 *tw3, int tw3_count,
    const hcbudoux_impl_item3 *tw4, int tw4_count, uint32_t utf32_prev3, uint32_t utf32_prev2, uint32_t utf32_prev1,
    uint32_t utf32_current, uint32_t utf32_next1, uint32_t utf32_next2) {
  const int sum = hcbudoux_impl_find1(uw1, uw1_count, utf32_prev3) +                              //
                  hcbudoux_impl_find1(uw2, uw2_count, utf32_prev2) +                              //
                  hcbudoux_impl_find1(uw3, uw3_count, utf32_prev1) +                              //
                  hcbudoux_impl_find1(uw4, uw4_count, utf32_current) +                            //
                  hcbudoux_impl_find1(uw5, uw5_count, utf32_next1) +                              //
                  hcbudoux_impl_find1(uw6, uw6_count, utf32_next2) +                              //
                  hcbudoux_impl_find2(bw1, bw1_count, utf32_prev2, utf32_prev1) +                 //
                  hcbudoux_impl_find2(bw2, bw2_count, utf32_prev1, utf32_current) +               //
                  hcbudoux_impl_find2(bw3, bw3_count, utf32_current, utf32_next1) +               //
                  hcbudoux_impl_find3(tw1, tw1_count, utf32_prev3, utf32_prev2, utf32_prev1) +    //
                  hcbudoux_impl_find3(tw2, tw2_count, utf32_prev2, utf32_prev1, utf32_current) +  //
                  hcbudoux_impl_find3(tw3, tw3_count, utf32_prev1, utf32_current, utf32_next1) +  //
                  hcbudoux_impl_find3(tw4, tw4_count, utf32_current, utf32_next1, utf32_next2);
  return base_score + 2 * sum;
}

//
// BudouX Models
//
#if defined(HCBUDOUX_USE_JA) && (HCBUDOUX_USE_JA)
static int hcbudoux_impl_compute_score_ja(uint32_t utf32_prev3, uint32_t utf32_prev2, uint32_t utf32_prev1,
                                          uint32_t utf32_current, uint32_t utf32_next1, uint32_t utf32_next2) {
  static const hcbudoux_impl_item1 uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW1)};
  static const hcbudoux_impl_item1 uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW2)};
  static const hcbudoux_impl_item1 uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW3)};
  static const hcbudoux_impl_item1 uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW4)};
  static const hcbudoux_impl_item1 uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW5)};
  static const hcbudoux_impl_item1 uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW6)};
  static const hcbudoux_impl_item2 bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.BW1)};
  static const hcbudoux_impl_item2 bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.BW2)};
  static const hcbudoux_impl_item2 bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.BW3)};
  static const hcbudoux_impl_item3 tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.TW1)};
  static const hcbudoux_impl_item3 tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.TW2)};
  static const hcbudoux_impl_item3 tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.TW3)};
  static const hcbudoux_impl_item3 tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.TW4)};
  static const int base = HCBUDOUX_IMPL_TEMPLATE(_ja_.Base);
  return hcbudoux_impl_compute_score_from_tables(
      base, uw1, (int)(sizeof(uw1) / sizeof(uw1[0])), uw2, (int)(sizeof(uw2) / sizeof(uw2[0])), uw3,
      (int)(sizeof(uw3) / sizeof(uw3[0])), uw4, (int)(sizeof(uw4) / sizeof(uw4[0])), uw5,
      (int)(sizeof(uw5) / sizeof(uw5[0])), uw6, (int)(sizeof(uw6) / sizeof(uw6[0])), bw1,
      (int)(sizeof(bw1) / sizeof(bw1[0])), bw2, (int)(sizeof(bw2) / sizeof(bw2[0])), bw3,
      (int)(sizeof(bw3) / sizeof(bw3[0])), tw1, (int)(sizeof(tw1) / sizeof(tw1[0])), tw2,
      (int)(sizeof(tw2) / sizeof(tw2[0])), tw3, (int)(sizeof(tw3) / sizeof(tw3[0])), tw4,
      (int)(sizeof(tw4) / sizeof(tw4[0])), utf32_prev3, utf32_prev2, utf32_prev1, utf32_current, utf32_next1,
      utf32_next2);
}
#endif

#if defined(HCBUDOUX_USE_JA_KNBC) && (HCBUDOUX_USE_JA_KNBC)
static int hcbudoux_impl_compute_score_ja_knbc(uint32_t utf32_prev3, uint32_t utf32_prev2, uint32_t utf32_prev1,
                                               uint32_t utf32_current, uint32_t utf32_next1, uint32_t utf32_next2) {
  static const hcbudoux_impl_item1 uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW1)};
  static const hcbudoux_impl_item1 uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW2)};
  static const hcbudoux_impl_item1 uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW3)};
  static const hcbudoux_impl_item1 uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW4)};
  static const hcbudoux_impl_item1 uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW5)};
  static const hcbudoux_impl_item1 uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW6)};
  static const hcbudoux_impl_item2 bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.BW1)};
  static const hcbudoux_impl_item2 bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.BW2)};
  static const hcbudoux_impl_item2 bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.BW3)};
  static const hcbudoux_impl_item3 tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.TW1)};
  static const hcbudoux_impl_item3 tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.TW2)};
  static const hcbudoux_impl_item3 tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.TW3)};
  static const hcbudoux_impl_item3 tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.TW4)};
  static const int base = HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.Base);
  return hcbudoux_impl_compute_score_from_tables(
      base, uw1, (int)(sizeof(uw1) / sizeof(uw1[0])), uw2, (int)(sizeof(uw2) / sizeof(uw2[0])), uw3,
      (int)(sizeof(uw3) / sizeof(uw3[0])), uw4, (int)(sizeof(uw4) / sizeof(uw4[0])), uw5,
      (int)(sizeof(uw5) / sizeof(uw5[0])), uw6, (int)(sizeof(uw6) / sizeof(uw6[0])), bw1,
      (int)(sizeof(bw1) / sizeof(bw1[0])), bw2, (int)(sizeof(bw2) / sizeof(bw2[0])), bw3,
      (int)(sizeof(bw3) / sizeof(bw3[0])), tw1, (int)(sizeof(tw1) / sizeof(tw1[0])), tw2,
      (int)(sizeof(tw2) / sizeof(tw2[0])), tw3, (int)(sizeof(tw3) / sizeof(tw3[0])), tw4,
      (int)(sizeof(tw4) / sizeof(tw4[0])), utf32_prev3, utf32_prev2, utf32_prev1, utf32_current, utf32_next1,
      utf32_next2);
}
#endif

#if defined(HCBUDOUX_USE_TH) && (HCBUDOUX_USE_TH)
static int hcbudoux_impl_compute_score_th(uint32_t utf32_prev3, uint32_t utf32_prev2, uint32_t utf32_prev1,
                                          uint32_t utf32_current, uint32_t utf32_next1, uint32_t utf32_next2) {
  static const hcbudoux_impl_item1 uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW1)};
  static const hcbudoux_impl_item1 uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW2)};
  static const hcbudoux_impl_item1 uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW3)};
  static const hcbudoux_impl_item1 uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW4)};
  static const hcbudoux_impl_item1 uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW5)};
  static const hcbudoux_impl_item1 uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW6)};
  static const hcbudoux_impl_item2 bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.BW1)};
  static const hcbudoux_impl_item2 bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.BW2)};
  static const hcbudoux_impl_item2 bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.BW3)};
  static const hcbudoux_impl_item3 tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.TW1)};
  static const hcbudoux_impl_item3 tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.TW2)};
  static const hcbudoux_impl_item3 tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.TW3)};
  static const hcbudoux_impl_item3 tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.TW4)};
  static const int base = HCBUDOUX_IMPL_TEMPLATE(_th_.Base);
  return hcbudoux_impl_compute_score_from_tables(
      base, uw1, (int)(sizeof(uw1) / sizeof(uw1[0])), uw2, (int)(sizeof(uw2) / sizeof(uw2[0])), uw3,
      (int)(sizeof(uw3) / sizeof(uw3[0])), uw4, (int)(sizeof(uw4) / sizeof(uw4[0])), uw5,
      (int)(sizeof(uw5) / sizeof(uw5[0])), uw6, (int)(sizeof(uw6) / sizeof(uw6[0])), bw1,
      (int)(sizeof(bw1) / sizeof(bw1[0])), bw2, (int)(sizeof(bw2) / sizeof(bw2[0])), bw3,
      (int)(sizeof(bw3) / sizeof(bw3[0])), tw1, (int)(sizeof(tw1) / sizeof(tw1[0])), tw2,
      (int)(sizeof(tw2) / sizeof(tw2[0])), tw3, (int)(sizeof(tw3) / sizeof(tw3[0])), tw4,
      (int)(sizeof(tw4) / sizeof(tw4[0])), utf32_prev3, utf32_prev2, utf32_prev1, utf32_current, utf32_next1,
      utf32_next2);
}
#endif

#if defined(HCBUDOUX_USE_ZH_HANS) && (HCBUDOUX_USE_ZH_HANS)
static int hcbudoux_impl_compute_score_zh_hans(uint32_t utf32_prev3, uint32_t utf32_prev2, uint32_t utf32_prev1,
                                               uint32_t utf32_current, uint32_t utf32_next1, uint32_t utf32_next2) {
  static const hcbudoux_impl_item1 uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW1)};
  static const hcbudoux_impl_item1 uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW2)};
  static const hcbudoux_impl_item1 uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW3)};
  static const hcbudoux_impl_item1 uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW4)};
  static const hcbudoux_impl_item1 uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW5)};
  static const hcbudoux_impl_item1 uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW6)};
  static const hcbudoux_impl_item2 bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.BW1)};
  static const hcbudoux_impl_item2 bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.BW2)};
  static const hcbudoux_impl_item2 bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.BW3)};
  static const hcbudoux_impl_item3 tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.TW1)};
  static const hcbudoux_impl_item3 tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.TW2)};
  static const hcbudoux_impl_item3 tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.TW3)};
  static const hcbudoux_impl_item3 tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.TW4)};
  static const int base = HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.Base);
  return hcbudoux_impl_compute_score_from_tables(
      base, uw1, (int)(sizeof(uw1) / sizeof(uw1[0])), uw2, (int)(sizeof(uw2) / sizeof(uw2[0])), uw3,
      (int)(sizeof(uw3) / sizeof(uw3[0])), uw4, (int)(sizeof(uw4) / sizeof(uw4[0])), uw5,
      (int)(sizeof(uw5) / sizeof(uw5[0])), uw6, (int)(sizeof(uw6) / sizeof(uw6[0])), bw1,
      (int)(sizeof(bw1) / sizeof(bw1[0])), bw2, (int)(sizeof(bw2) / sizeof(bw2[0])), bw3,
      (int)(sizeof(bw3) / sizeof(bw3[0])), tw1, (int)(sizeof(tw1) / sizeof(tw1[0])), tw2,
      (int)(sizeof(tw2) / sizeof(tw2[0])), tw3, (int)(sizeof(tw3) / sizeof(tw3[0])), tw4,
      (int)(sizeof(tw4) / sizeof(tw4[0])), utf32_prev3, utf32_prev2, utf32_prev1, utf32_current, utf32_next1,
      utf32_next2);
}
#endif

#if defined(HCBUDOUX_USE_ZH_HANT) && (HCBUDOUX_USE_ZH_HANT)
static int hcbudoux_impl_compute_score_zh_hant(uint32_t utf32_prev3, uint32_t utf32_prev2, uint32_t utf32_prev1,
                                               uint32_t utf32_current, uint32_t utf32_next1, uint32_t utf32_next2) {
  static const hcbudoux_impl_item1 uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW1)};
  static const hcbudoux_impl_item1 uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW2)};
  static const hcbudoux_impl_item1 uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW3)};
  static const hcbudoux_impl_item1 uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW4)};
  static const hcbudoux_impl_item1 uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW5)};
  static const hcbudoux_impl_item1 uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW6)};
  static const hcbudoux_impl_item2 bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.BW1)};
  static const hcbudoux_impl_item2 bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.BW2)};
  static const hcbudoux_impl_item2 bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.BW3)};
  static const hcbudoux_impl_item3 tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.TW1)};
  static const hcbudoux_impl_item3 tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.TW2)};
  static const hcbudoux_impl_item3 tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.TW3)};
  static const hcbudoux_impl_item3 tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.TW4)};
  static const int base = HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.Base);
  return hcbudoux_impl_compute_score_from_tables(
      base, uw1, (int)(sizeof(uw1) / sizeof(uw1[0])), uw2, (int)(sizeof(uw2) / sizeof(uw2[0])), uw3,
      (int)(sizeof(uw3) / sizeof(uw3[0])), uw4, (int)(sizeof(uw4) / sizeof(uw4[0])), uw5,
      (int)(sizeof(uw5) / sizeof(uw5[0])), uw6, (int)(sizeof(uw6) / sizeof(uw6[0])), bw1,
      (int)(sizeof(bw1) / sizeof(bw1[0])), bw2, (int)(sizeof(bw2) / sizeof(bw2[0])), bw3,
      (int)(sizeof(bw3) / sizeof(bw3[0])), tw1, (int)(sizeof(tw1) / sizeof(tw1[0])), tw2,
      (int)(sizeof(tw2) / sizeof(tw2[0])), tw3, (int)(sizeof(tw3) / sizeof(tw3[0])), tw4,
      (int)(sizeof(tw4) / sizeof(tw4[0])), utf32_prev3, utf32_prev2, utf32_prev1, utf32_current, utf32_next1,
      utf32_next2);
}
#endif

//
// Score computation
//
static int hcbudoux_impl_compute_score(hcbudoux_ctx *ctx, hcbudoux_impl_lang lang) {
  const uint32_t u0 = ctx->impl.utf32s[0];
  const uint32_t u1 = ctx->impl.utf32s[1];
  const uint32_t u2 = ctx->impl.utf32s[2];
  const uint32_t u3 = ctx->impl.utf32s[3];
  const uint32_t u4 = ctx->impl.utf32s[4];
  const uint32_t u5 = ctx->impl.utf32s[5];

  switch (lang) {
    case hcbudoux_impl_lang_ja:
#if defined(HCBUDOUX_USE_JA) && (HCBUDOUX_USE_JA)
      return hcbudoux_impl_compute_score_ja(u0, u1, u2, u3, u4, u5);
#endif
      break;
    case hcbudoux_impl_lang_ja_knbc:
#if defined(HCBUDOUX_USE_JA_KNBC) && (HCBUDOUX_USE_JA_KNBC)
      return hcbudoux_impl_compute_score_ja_knbc(u0, u1, u2, u3, u4, u5);
#endif
      break;
    case hcbudoux_impl_lang_th:
#if defined(HCBUDOUX_USE_TH) && (HCBUDOUX_USE_TH)
      return hcbudoux_impl_compute_score_th(u0, u1, u2, u3, u4, u5);
#endif
      break;
    case hcbudoux_impl_lang_zh_hans:
#if defined(HCBUDOUX_USE_ZH_HANS) && (HCBUDOUX_USE_ZH_HANS)
      return hcbudoux_impl_compute_score_zh_hans(u0, u1, u2, u3, u4, u5);
#endif
      break;
    case hcbudoux_impl_lang_zh_hant:
#if defined(HCBUDOUX_USE_ZH_HANT) && (HCBUDOUX_USE_ZH_HANT)
      return hcbudoux_impl_compute_score_zh_hant(u0, u1, u2, u3, u4, u5);
#endif
      break;
    default:
      break;
  }
  return -1;
}

//
// Get next string view
//
static bool hcbudoux_impl_getnext(hcbudoux_ctx *ctx, hcbudoux_span *span, hcbudoux_impl_lang lang) {
  for (;;) {
    // Read a UTF-8 character
    uint32_t new_utf32_char = 0;
    const int new_utf32_char_index = ctx->impl.curr_index;
    int new_utf32_char_size_in_bytes = 0;
    {
      const int rest = ctx->impl.utf8_str_size_in_bytes - new_utf32_char_index;

      const uint8_t c0 = (uint8_t)(rest >= 1 ? (ctx->impl.utf8_str)[new_utf32_char_index + 0] : 0);
      const uint8_t c1 = (uint8_t)(rest >= 2 ? (ctx->impl.utf8_str)[new_utf32_char_index + 1] : 0);
      const uint8_t c2 = (uint8_t)(rest >= 3 ? (ctx->impl.utf8_str)[new_utf32_char_index + 2] : 0);
      const uint8_t c3 = (uint8_t)(rest >= 4 ? (ctx->impl.utf8_str)[new_utf32_char_index + 3] : 0);

      // https://en.wikipedia.org/wiki/UTF-8#Description
      //      byte1
      //      0yyyzzzz    1 byte      x & 0x80 == 0
      //      110xxxyy    2 bytes     x & 0xe0 == 0xc0
      //      1110wwww    3 bytes     x & 0xf0 == 0xe0
      //      11110uvv    4 bytes     x & 0xf8 == 0xf0
      if ((c0 & 0x80) == 0) {
        if (rest >= 1) {
          // c0
          // 0yyyzzz
          //
          // |         |         |         |         |
          // |0000 0000|0000 0000|0000 0000|0yyy zzzz|    [0x0000,0x007f]
          const uint32_t p0 = c0 & 0x7f;
          const uint32_t code_point = p0;
          new_utf32_char = code_point;
          new_utf32_char_size_in_bytes = 1;
        }
      } else if ((c0 & 0xe0) == 0xc0) {
        if (rest >= 2) {
          // c0       c1
          // 110xxxyy 10yyzzzz
          //
          // |         |         |         |         |
          // |0000 0000|0000 0000|0000 0xxx|yyyy zzzz|    [0x0080,0x07ff]
          const uint32_t p0 = (c0 & 0x1f) << 6;
          const uint32_t p1 = (c1 & 0x3f);
          const uint32_t code_point = p0 | p1;
          new_utf32_char = code_point;
          new_utf32_char_size_in_bytes = 2;
        }
      } else if ((c0 & 0xf0) == 0xe0) {
        if (rest >= 3) {
          // c0       c1       c2
          // 1110wwww 10xxxxyy 10yyzzzz
          //
          // |         |         |         |         |
          // |0000 0000|0000 0000|wwww xxxx|yyyy zzzz|    [0x0800,0xffff]
          const uint32_t p0 = (c0 & 0x0f) << 12;
          const uint32_t p1 = (c1 & 0x3f) << 6;
          const uint32_t p2 = (c2 & 0x3f);
          const uint32_t code_point = p0 | p1 | p2;
          new_utf32_char = code_point;
          new_utf32_char_size_in_bytes = 3;
        }
      } else if ((c0 & 0xf8) == 0xf0) {
        if (rest >= 4) {
          // c0       c1       c2       c3
          // 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
          //
          // |         |         |         |         |
          // |0000 0000|000u vvvv|wwww xxxx|yyyy zzzz|    [0x010000,0x10ffff]
          const uint32_t p0 = (c0 & 0x07) << 18;
          const uint32_t p1 = (c1 & 0x3f) << 12;
          const uint32_t p2 = (c2 & 0x3f) << 6;
          const uint32_t p3 = (c3 & 0x3f);
          const uint32_t code_point = p0 | p1 | p2 | p3;
          new_utf32_char = code_point;
          new_utf32_char_size_in_bytes = 4;
        }
      }
    }

    // Advance at least 1 byte on invalid UTF-8
    if (new_utf32_char_size_in_bytes == 0) {
      new_utf32_char_size_in_bytes = 1;
    }

    // Add new UTF32 character to the queue
    ctx->impl.utf32s[0] = ctx->impl.utf32s[1];
    ctx->impl.utf32s[1] = ctx->impl.utf32s[2];
    ctx->impl.utf32s[2] = ctx->impl.utf32s[3];
    ctx->impl.utf32s[3] = ctx->impl.utf32s[4];
    ctx->impl.utf32s[4] = ctx->impl.utf32s[5];
    ctx->impl.utf32s[5] = new_utf32_char;

    ctx->impl.indices[0] = ctx->impl.indices[1];
    ctx->impl.indices[1] = ctx->impl.indices[2];
    ctx->impl.indices[2] = ctx->impl.indices[3];
    ctx->impl.indices[3] = ctx->impl.indices[4];
    ctx->impl.indices[4] = ctx->impl.indices[5];
    ctx->impl.indices[5] = new_utf32_char_index;

    ctx->impl.curr_index += new_utf32_char_size_in_bytes;

    {
      const int start = ctx->impl.last_index;
      const int end = ctx->impl.indices[3];
      const int length = end - start;

      // indices[3] (end) is the byte offset of the current character (utf32s[3]).
      // -1 means the slot has not been filled yet; an offset past the end means EOF padding.
      // The code point itself is not tested here, so U+0000 and invalid bytes (which decode
      // to 0) are scored like any other character.  No model key contains U+0000.
      if (end >= 0 && end < ctx->impl.utf8_str_size_in_bytes) {
        // Queue contains valid input.

        // Evaluate queue
        const int score = hcbudoux_impl_compute_score(ctx, lang);

        // If score > 0, it means we can put &nbsp; between character at
        // utf32s[2] and utf32s[3]. Also, since the first valid character may
        // have positive score, we should avoid it (length <= 0).
        if (score <= 0 || length <= 0) {
          continue;
        }

        span->offset = start;
        span->length = length;
        ctx->impl.last_index = end;
        return true;  // true indicates valid span
      } else if (end >= ctx->impl.utf8_str_size_in_bytes) {
        // Queue is empty. (index exceeded the last character)

        if (length <= 0 || start >= ctx->impl.utf8_str_size_in_bytes) {
          break;
        }

        // Return the last valid chunk before entering EOF state.
        span->offset = start;
        span->length = length;
        ctx->impl.curr_index = ctx->impl.utf8_str_size_in_bytes;
        ctx->impl.last_index = ctx->impl.utf8_str_size_in_bytes;
        return true;  // true indicates valid span
      }
    }
  }

  // Return empty span.
  span->offset = 0;
  span->length = 0;
  return false;  // false indicates invalid span, end of string
}

//
// Public API: Get next string view
//
#if defined(HCBUDOUX_USE_JA) && (HCBUDOUX_USE_JA)
bool hcbudoux_getnext_ja(hcbudoux_ctx *ctx, hcbudoux_span *span) {
  return hcbudoux_impl_getnext(ctx, span, hcbudoux_impl_lang_ja);
}
#endif

#if defined(HCBUDOUX_USE_JA_KNBC) && (HCBUDOUX_USE_JA_KNBC)
bool hcbudoux_getnext_ja_knbc(hcbudoux_ctx *ctx, hcbudoux_span *span) {
  return hcbudoux_impl_getnext(ctx, span, hcbudoux_impl_lang_ja_knbc);
}
#endif

#if defined(HCBUDOUX_USE_TH) && (HCBUDOUX_USE_TH)
bool hcbudoux_getnext_th(hcbudoux_ctx *ctx, hcbudoux_span *span) {
  return hcbudoux_impl_getnext(ctx, span, hcbudoux_impl_lang_th);
}
#endif

#if defined(HCBUDOUX_USE_ZH_HANS) && (HCBUDOUX_USE_ZH_HANS)
bool hcbudoux_getnext_zh_hans(hcbudoux_ctx *ctx, hcbudoux_span *span) {
  return hcbudoux_impl_getnext(ctx, span, hcbudoux_impl_lang_zh_hans);
}
#endif

#if defined(HCBUDOUX_USE_ZH_HANT) && (HCBUDOUX_USE_ZH_HANT)
bool hcbudoux_getnext_zh_hant(hcbudoux_ctx *ctx, hcbudoux_span *span) {
  return hcbudoux_impl_getnext(ctx, span, hcbudoux_impl_lang_zh_hant);
}
#endif
#endif  // defined(HCBUDOUX_IMPLEMENTATION)

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // HCBUDOUX_H_INCLUDED

//
// Apache License 2.0 (applies to the "BudouX Models" section)
// ===========================================================
//
//
//                                  Apache License
//                            Version 2.0, January 2004
//                         http://www.apache.org/licenses/
//
//    TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
//    1. Definitions.
//
//       "License" shall mean the terms and conditions for use, reproduction,
//       and distribution as defined by Sections 1 through 9 of this document.
//
//       "Licensor" shall mean the copyright owner or entity authorized by
//       the copyright owner that is granting the License.
//
//       "Legal Entity" shall mean the union of the acting entity and all
//       other entities that control, are controlled by, or are under common
//       control with that entity. For the purposes of this definition,
//       "control" means (i) the power, direct or indirect, to cause the
//       direction or management of such entity, whether by contract or
//       otherwise, or (ii) ownership of fifty percent (50%) or more of the
//       outstanding shares, or (iii) beneficial ownership of such entity.
//
//       "You" (or "Your") shall mean an individual or Legal Entity
//       exercising permissions granted by this License.
//
//       "Source" form shall mean the preferred form for making modifications,
//       including but not limited to software source code, documentation
//       source, and configuration files.
//
//       "Object" form shall mean any form resulting from mechanical
//       transformation or translation of a Source form, including but
//       not limited to compiled object code, generated documentation,
//       and conversions to other media types.
//
//       "Work" shall mean the work of authorship, whether in Source or
//       Object form, made available under the License, as indicated by a
//       copyright notice that is included in or attached to the work
//       (an example is provided in the Appendix below).
//
//       "Derivative Works" shall mean any work, whether in Source or Object
//       form, that is based on (or derived from) the Work and for which the
//       editorial revisions, annotations, elaborations, or other modifications
//       represent, as a whole, an original work of authorship. For the purposes
//       of this License, Derivative Works shall not include works that remain
//       separable from, or merely link (or bind by name) to the interfaces of,
//       the Work and Derivative Works thereof.
//
//       "Contribution" shall mean any work of authorship, including
//       the original version of the Work and any modifications or additions
//       to that Work or Derivative Works thereof, that is intentionally
//       submitted to Licensor for inclusion in the Work by the copyright owner
//       or by an individual or Legal Entity authorized to submit on behalf of
//       the copyright owner. For the purposes of this definition, "submitted"
//       means any form of electronic, verbal, or written communication sent
//       to the Licensor or its representatives, including but not limited to
//       communication on electronic mailing lists, source code control systems,
//       and issue tracking systems that are managed by, or on behalf of, the
//       Licensor for the purpose of discussing and improving the Work, but
//       excluding communication that is conspicuously marked or otherwise
//       designated in writing by the copyright owner as "Not a Contribution."
//
//       "Contributor" shall mean Licensor and any individual or Legal Entity
//       on behalf of whom a Contribution has been received by Licensor and
//       subsequently incorporated within the Work.
//
//    2. Grant of Copyright License. Subject to the terms and conditions of
//       this License, each Contributor hereby grants to You a perpetual,
//       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
//       copyright license to reproduce, prepare Derivative Works of,
//       publicly display, publicly perform, sublicense, and distribute the
//       Work and such Derivative Works in Source or Object form.
//
//    3. Grant of Patent License. Subject to the terms and conditions of
//       this License, each Contributor hereby grants to You a perpetual,
//       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
//       (except as stated in this section) patent license to make, have made,
//       use, offer to sell, sell, import, and otherwise transfer the Work,
//       where such license applies only to those patent claims licensable
//       by such Contributor that are necessarily infringed by their
//       Contribution(s) alone or by combination of their Contribution(s)
//       with the Work to which such Contribution(s) was submitted. If You
//       institute patent litigation against any entity (including a
//       cross-claim or counterclaim in a lawsuit) alleging that the Work
//       or a Contribution incorporated within the Work constitutes direct
//       or contributory patent infringement, then any patent licenses
//       granted to You under this License for that Work shall terminate
//       as of the date such litigation is filed.
//
//    4. Redistribution. You may reproduce and distribute copies of the
//       Work or Derivative Works thereof in any medium, with or without
//       modifications, and in Source or Object form, provided that You
//       meet the following conditions:
//
//       (a) You must give any other recipients of the Work or
//           Derivative Works a copy of this License; and
//
//       (b) You must cause any modified files to carry prominent notices
//           stating that You changed the files; and
//
//       (c) You must retain, in the Source form of any Derivative Works
//           that You distribute, all copyright, patent, trademark, and
//           attribution notices from the Source form of the Work,
//           excluding those notices that do not pertain to any part of
//           the Derivative Works; and
//
//       (d) If the Work includes a "NOTICE" text file as part of its
//           distribution, then any Derivative Works that You distribute must
//           include a readable copy of the attribution notices contained
//           within such NOTICE file, excluding those notices that do not
//           pertain to any part of the Derivative Works, in at least one
//           of the following places: within a NOTICE text file distributed
//           as part of the Derivative Works; within the Source form or
//           documentation, if provided along with the Derivative Works; or,
//           within a display generated by the Derivative Works, if and
//           wherever such third-party notices normally appear. The contents
//           of the NOTICE file are for informational purposes only and
//           do not modify the License. You may add Your own attribution
//           notices within Derivative Works that You distribute, alongside
//           or as an addendum to the NOTICE text from the Work, provided
//           that such additional attribution notices cannot be construed
//           as modifying the License.
//
//       You may add Your own copyright statement to Your modifications and
//       may provide additional or different license terms and conditions
//       for use, reproduction, or distribution of Your modifications, or
//       for any such Derivative Works as a whole, provided Your use,
//       reproduction, and distribution of the Work otherwise complies with
//       the conditions stated in this License.
//
//    5. Submission of Contributions. Unless You explicitly state otherwise,
//       any Contribution intentionally submitted for inclusion in the Work
//       by You to the Licensor shall be under the terms and conditions of
//       this License, without any additional terms or conditions.
//       Notwithstanding the above, nothing herein shall supersede or modify
//       the terms of any separate license agreement you may have executed
//       with Licensor regarding such Contributions.
//
//    6. Trademarks. This License does not grant permission to use the trade
//       names, trademarks, service marks, or product names of the Licensor,
//       except as required for reasonable and customary use in describing the
//       origin of the Work and reproducing the content of the NOTICE file.
//
//    7. Disclaimer of Warranty. Unless required by applicable law or
//       agreed to in writing, Licensor provides the Work (and each
//       Contributor provides its Contributions) on an "AS IS" BASIS,
//       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//       implied, including, without limitation, any warranties or conditions
//       of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
//       PARTICULAR PURPOSE. You are solely responsible for determining the
//       appropriateness of using or redistributing the Work and assume any
//       risks associated with Your exercise of permissions under this License.
//
//    8. Limitation of Liability. In no event and under no legal theory,
//       whether in tort (including negligence), contract, or otherwise,
//       unless required by applicable law (such as deliberate and grossly
//       negligent acts) or agreed to in writing, shall any Contributor be
//       liable to You for damages, including any direct, indirect, special,
//       incidental, or consequential damages of any character arising as a
//       result of this License or out of the use or inability to use the
//       Work (including but not limited to damages for loss of goodwill,
//       work stoppage, computer failure or malfunction, or any and all
//       other commercial damages or losses), even if such Contributor
//       has been advised of the possibility of such damages.
//
//    9. Accepting Warranty or Additional Liability. While redistributing
//       the Work or Derivative Works thereof, You may choose to offer,
//       and charge a fee for, acceptance of support, warranty, indemnity,
//       or other liability obligations and/or rights consistent with this
//       License. However, in accepting such obligations, You may act only
//       on Your own behalf and on Your sole responsibility, not on behalf
//       of any other Contributor, and only if You agree to indemnify,
//       defend, and hold each Contributor harmless for any liability
//       incurred by, or claims asserted against, such Contributor by reason
//       of your accepting any such warranty or additional liability.
//
//    END OF TERMS AND CONDITIONS
//
//    APPENDIX: How to apply the Apache License to your work.
//
//       To apply the Apache License to your work, attach the following
//       boilerplate notice, with the fields enclosed by brackets "[]"
//       replaced with your own identifying information. (Don't include
//       the brackets!)  The text should be enclosed in the appropriate
//       comment syntax for the file format. We also recommend that a
//       file or class name and description of purpose be included on the
//       same "printed page" as the copyright notice for easier
//       identification within third-party archives.
//
//    Copyright [yyyy] [name of copyright owner]
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
