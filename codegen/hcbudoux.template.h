// hcbudoux.h
// ==========
//
// Header-only C11 implementation of BudouX line break library for Chinese, Japanese and Thai language.
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
// MSVC: cl.exe /utf-8 /nologo -I include example1.c
//
//     ```C
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
//         static char const utf8Str[]  = u8"次の決闘がまもなく始まる！";
//         int const         utf8Strlen = (int)strlen(utf8Str);
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
// SPDX-License-Identifier: CC0-1.0
//
// "hcbudoux.h" by Takayuki Matsuoka.
// To the extent possible under law, the person who associated CC0-1.0 with
// "hcbudoux.h". See https://creativecommons.org/publicdomain/zero/1.0/ for
// CC0-1.0 legalcode.

#ifndef HCBUDOUX_H_INCLUDED
#define HCBUDOUX_H_INCLUDED 1

#ifndef HCBUDOUX_DONT_INCLUDE_STD
#include <stdbool.h>  // bool, true, false
#include <stdint.h>   // uint8_t, uint32_t, uint64_t
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
  hcbudoux_version_minor = 1,
  hcbudoux_version_patch = 0,
};

typedef struct hcbudoux_ctx {
  struct {
    const uint8_t *utf8_str;
    int utf8_str_size_in_bytes;
    int curr_index;
    int last_index;
    uint32_t utf32s[6];
    int indices[6];
  } impl;
} hcbudoux_ctx;

typedef struct hcbudoux_span {
  int offset;  // public member: Offset in bytes from the beginning of utf8_str.
  int length;  // public member: Length in bytes.
} hcbudoux_span;

// Initialize a parser context with a UTF-8 string.
// utf8_str is encoded in UTF-8.  The lifetime of utf8_str is longer than hcbudoux_ctx.
// We don't need to "close" hcbudoux_ctx since it doesn't allocate dynamic resources.
// hcbudoux doesn't require that utf8_str is terminated with '\0'.
void hcbudoux_init(hcbudoux_ctx *ctx, const void *utf8_str, int utf8_str_size_in_bytes);

// Get the next string view of the specific language.
// Returns false when the parser reaches the end of utf8_str.
// When it returns true, span contains a valid offset and length of the string view.
//
// The string view is not terminated with `\0'.
// Use span->length to terminate the string view properly.
bool hcbudoux_getnext_ja(hcbudoux_ctx *ctx, hcbudoux_span *span);
bool hcbudoux_getnext_ja_knbc(hcbudoux_ctx *ctx, hcbudoux_span *span);
bool hcbudoux_getnext_th(hcbudoux_ctx *ctx, hcbudoux_span *span);
bool hcbudoux_getnext_zh_hans(hcbudoux_ctx *ctx, hcbudoux_span *span);
bool hcbudoux_getnext_zh_hant(hcbudoux_ctx *ctx, hcbudoux_span *span);

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
void hcbudoux_init(hcbudoux_ctx *ctx, const void *utf8_str, int utf8_str_size_in_bytes) {
  ctx->impl.utf8_str = (const uint8_t *)utf8_str;
  ctx->impl.utf8_str_size_in_bytes = utf8_str_size_in_bytes;
  ctx->impl.curr_index = 0;
  ctx->impl.last_index = 0;
  for (int i = 0, n = (int)(sizeof(ctx->impl.utf32s) / sizeof(ctx->impl.utf32s[0])); i < n; ++i) {
    ctx->impl.utf32s[i] = 0;
  }
  for (int i = 0, n = (int)(sizeof(ctx->impl.indices) / sizeof(ctx->impl.indices[0])); i < n; ++i) {
    ctx->impl.indices[i] = 0;
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
  while (len > 1) {
    int const half = len / 2;
    base += (base[half - 1].var < x) * half;
    len -= half;
  }
  return base->var == x ? base->score : 0;
}

static int hcbudoux_impl_find2(const hcbudoux_impl_item2 *base, int len, uint32_t x0, uint32_t x1) {
  uint64_t const x = ((uint64_t)x1) | (((uint64_t)x0) << 21);
  while (len > 1) {
    int const half = len / 2;
    base += (base[half - 1].var < x) * half;
    len -= half;
  }
  return base->var == x ? base->score : 0;
}

static int hcbudoux_impl_find3(const hcbudoux_impl_item3 *base, int len, uint32_t x0, uint32_t x1, uint32_t x2) {
  uint64_t const x = ((uint64_t)x2) | (((uint64_t)x1) << 21) | (((uint64_t)x0) << 42);
  while (len > 1) {
    int const half = len / 2;
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
  int const sum = hcbudoux_impl_find1(uw1, uw1_count, utf32_prev3) + hcbudoux_impl_find1(uw2, uw2_count, utf32_prev2) +
                  hcbudoux_impl_find1(uw3, uw3_count, utf32_prev1) +
                  hcbudoux_impl_find1(uw4, uw4_count, utf32_current) +
                  hcbudoux_impl_find1(uw5, uw5_count, utf32_next1) + hcbudoux_impl_find1(uw6, uw6_count, utf32_next2) +
                  hcbudoux_impl_find2(bw1, bw1_count, utf32_prev2, utf32_prev1) +
                  hcbudoux_impl_find2(bw2, bw2_count, utf32_prev1, utf32_current) +
                  hcbudoux_impl_find2(bw3, bw3_count, utf32_current, utf32_next1) +
                  hcbudoux_impl_find3(tw1, tw1_count, utf32_prev3, utf32_prev2, utf32_prev1) +
                  hcbudoux_impl_find3(tw2, tw2_count, utf32_prev2, utf32_prev1, utf32_current) +
                  hcbudoux_impl_find3(tw3, tw3_count, utf32_prev1, utf32_current, utf32_next1) +
                  hcbudoux_impl_find3(tw4, tw4_count, utf32_current, utf32_next1, utf32_next2);
  return base_score + 2 * sum;
}

//
// BudouX Models
//
#if defined(HCBUDOUX_USE_JA) && (HCBUDOUX_USE_JA)
static int hcbudoux_impl_compute_score_ja(uint32_t utf32_prev3, uint32_t utf32_prev2, uint32_t utf32_prev1,
                                          uint32_t utf32_current, uint32_t utf32_next1, uint32_t utf32_next2) {
  static hcbudoux_impl_item1 const uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW1)};
  static hcbudoux_impl_item1 const uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW2)};
  static hcbudoux_impl_item1 const uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW3)};
  static hcbudoux_impl_item1 const uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW4)};
  static hcbudoux_impl_item1 const uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW5)};
  static hcbudoux_impl_item1 const uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.UW6)};
  static hcbudoux_impl_item2 const bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.BW1)};
  static hcbudoux_impl_item2 const bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.BW2)};
  static hcbudoux_impl_item2 const bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.BW3)};
  static hcbudoux_impl_item3 const tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.TW1)};
  static hcbudoux_impl_item3 const tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.TW2)};
  static hcbudoux_impl_item3 const tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.TW3)};
  static hcbudoux_impl_item3 const tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_.TW4)};
  static int const base = HCBUDOUX_IMPL_TEMPLATE(_ja_.Base);
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
  static hcbudoux_impl_item1 const uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW1)};
  static hcbudoux_impl_item1 const uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW2)};
  static hcbudoux_impl_item1 const uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW3)};
  static hcbudoux_impl_item1 const uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW4)};
  static hcbudoux_impl_item1 const uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW5)};
  static hcbudoux_impl_item1 const uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.UW6)};
  static hcbudoux_impl_item2 const bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.BW1)};
  static hcbudoux_impl_item2 const bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.BW2)};
  static hcbudoux_impl_item2 const bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.BW3)};
  static hcbudoux_impl_item3 const tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.TW1)};
  static hcbudoux_impl_item3 const tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.TW2)};
  static hcbudoux_impl_item3 const tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.TW3)};
  static hcbudoux_impl_item3 const tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.TW4)};
  static int const base = HCBUDOUX_IMPL_TEMPLATE(_ja_knbc_.Base);
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
  static hcbudoux_impl_item1 const uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW1)};
  static hcbudoux_impl_item1 const uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW2)};
  static hcbudoux_impl_item1 const uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW3)};
  static hcbudoux_impl_item1 const uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW4)};
  static hcbudoux_impl_item1 const uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW5)};
  static hcbudoux_impl_item1 const uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.UW6)};
  static hcbudoux_impl_item2 const bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.BW1)};
  static hcbudoux_impl_item2 const bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.BW2)};
  static hcbudoux_impl_item2 const bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.BW3)};
  static hcbudoux_impl_item3 const tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.TW1)};
  static hcbudoux_impl_item3 const tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.TW2)};
  static hcbudoux_impl_item3 const tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.TW3)};
  static hcbudoux_impl_item3 const tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_th_.TW4)};
  static int const base = HCBUDOUX_IMPL_TEMPLATE(_th_.Base);
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
  static hcbudoux_impl_item1 const uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW1)};
  static hcbudoux_impl_item1 const uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW2)};
  static hcbudoux_impl_item1 const uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW3)};
  static hcbudoux_impl_item1 const uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW4)};
  static hcbudoux_impl_item1 const uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW5)};
  static hcbudoux_impl_item1 const uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.UW6)};
  static hcbudoux_impl_item2 const bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.BW1)};
  static hcbudoux_impl_item2 const bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.BW2)};
  static hcbudoux_impl_item2 const bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.BW3)};
  static hcbudoux_impl_item3 const tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.TW1)};
  static hcbudoux_impl_item3 const tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.TW2)};
  static hcbudoux_impl_item3 const tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.TW3)};
  static hcbudoux_impl_item3 const tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.TW4)};
  static int const base = HCBUDOUX_IMPL_TEMPLATE(_zh_hans_.Base);
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
  static hcbudoux_impl_item1 const uw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW1)};
  static hcbudoux_impl_item1 const uw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW2)};
  static hcbudoux_impl_item1 const uw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW3)};
  static hcbudoux_impl_item1 const uw4[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW4)};
  static hcbudoux_impl_item1 const uw5[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW5)};
  static hcbudoux_impl_item1 const uw6[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.UW6)};
  static hcbudoux_impl_item2 const bw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.BW1)};
  static hcbudoux_impl_item2 const bw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.BW2)};
  static hcbudoux_impl_item2 const bw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.BW3)};
  static hcbudoux_impl_item3 const tw1[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.TW1)};
  static hcbudoux_impl_item3 const tw2[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.TW2)};
  static hcbudoux_impl_item3 const tw3[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.TW3)};
  static hcbudoux_impl_item3 const tw4[] = {HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.TW4)};
  static int const base = HCBUDOUX_IMPL_TEMPLATE(_zh_hant_.Base);
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
  uint32_t const u0 = ctx->impl.utf32s[0];
  uint32_t const u1 = ctx->impl.utf32s[1];
  uint32_t const u2 = ctx->impl.utf32s[2];
  uint32_t const u3 = ctx->impl.utf32s[3];
  uint32_t const u4 = ctx->impl.utf32s[4];
  uint32_t const u5 = ctx->impl.utf32s[5];

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
    int const new_utf32_char_index = ctx->impl.curr_index;
    int new_utf32_char_size_in_bytes = 0;
    {
      int const rest = ctx->impl.utf8_str_size_in_bytes - new_utf32_char_index;

      uint8_t const c0 = (uint8_t)(rest >= 1 ? (ctx->impl.utf8_str)[new_utf32_char_index + 0] : 0);
      uint8_t const c1 = (uint8_t)(rest >= 2 ? (ctx->impl.utf8_str)[new_utf32_char_index + 1] : 0);
      uint8_t const c2 = (uint8_t)(rest >= 3 ? (ctx->impl.utf8_str)[new_utf32_char_index + 2] : 0);
      uint8_t const c3 = (uint8_t)(rest >= 4 ? (ctx->impl.utf8_str)[new_utf32_char_index + 3] : 0);

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
          uint32_t const p0 = c0 & 0x7f;
          uint32_t const code_point = p0;
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
          uint32_t const p0 = (c0 & 0x1f) << 6;
          uint32_t const p1 = (c1 & 0x3f);
          uint32_t const code_point = p0 | p1;
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
          uint32_t const p0 = (c0 & 0x0f) << 12;
          uint32_t const p1 = (c1 & 0x3f) << 6;
          uint32_t const p2 = (c2 & 0x3f);
          uint32_t const code_point = p0 | p1 | p2;
          new_utf32_char = code_point;
          new_utf32_char_size_in_bytes = 3;
        }
      } else if ((c0 & 0xf8) == 0xf0) {
        if (rest >= 4) {
          // c0       c1       c2       c3
          // 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
          //
          // |         |         |         |         |
          // |0000 0000|000u vvvv|wwww xxxx|yyyy zzzz|    [0x010000,0x01ffff]
          uint32_t const p0 = (c0 & 0x07) << 18;
          uint32_t const p1 = (c1 & 0x3f) << 12;
          uint32_t const p2 = (c2 & 0x3f) << 6;
          uint32_t const p3 = (c3 & 0x3f);
          uint32_t const code_point = p0 | p1 | p2 | p3;
          new_utf32_char = code_point;
          new_utf32_char_size_in_bytes = 4;
        }
      }
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
      int const start = ctx->impl.last_index;
      int const end = ctx->impl.indices[3];
      int const length = end - start;

      // utf32s[3] represents 0 offset (current) character.
      if (ctx->impl.utf32s[3] != 0) {
        // Queue contains valid input.

        // Evaluate queue
        int const score = hcbudoux_impl_compute_score(ctx, lang);

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
      } else if (ctx->impl.indices[3] >= ctx->impl.utf8_str_size_in_bytes) {
        // Queue is empty. (index exceeded the last character)

        if (length <= 0 || start >= ctx->impl.utf8_str_size_in_bytes) {
          break;
        }

        // If we have the last valid chunk, return it before entering EOF state.
        if (length > 0 && start < ctx->impl.utf8_str_size_in_bytes) {
          // Set EOF state for next time.  Make sure we won't process further.
          span->offset = start;
          span->length = length;
          ctx->impl.curr_index = ctx->impl.utf8_str_size_in_bytes;
          ctx->impl.last_index = ctx->impl.utf8_str_size_in_bytes;
          return true;  // true indicates valid span
        }

        // There's nothing to do.  Return empty span.
        break;
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
#endif  // defined(HCBUDOUX_IMPL)

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // HCBUDOUX_H_INCLUDED
