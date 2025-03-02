// example2.c - Auto line break
#include <assert.h>  // static_assert
#include <stdio.h>   // printf
#include <string.h>  // strlen

#define HCBUDOUX_IMPLEMENTATION 1
#define HCBUDOUX_USE_JA 1  // Use model : ja
#include "east_asian_width.h"
#include "hcbudoux.h"
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#if defined(_WIN32)
void init(void) { SetConsoleOutputCP(CP_UTF8); }
#else
void init(void) {}
#endif

int computeStringWidth(const void *utf8_str, int utf8_str_size_in_bytes) {
  const char *const p = (const char *)utf8_str;
  int full_width_count = 0;
  int half_width_count = 0;
  int index = 0;
  while (index < utf8_str_size_in_bytes) {
    // Read a UTF-8 character
    uint32_t new_utf32_char = 0;
    int new_utf32_char_size_in_bytes = 0;
    {
      int const rest = utf8_str_size_in_bytes - index;

      uint8_t const c0 = (uint8_t)(rest >= 1 ? p[index + 0] : 0);
      uint8_t const c1 = (uint8_t)(rest >= 2 ? p[index + 1] : 0);
      uint8_t const c2 = (uint8_t)(rest >= 3 ? p[index + 2] : 0);
      uint8_t const c3 = (uint8_t)(rest >= 4 ? p[index + 3] : 0);

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

      index += new_utf32_char_size_in_bytes;

      int const eaw = east_asian_width(new_utf32_char);

      if (eaw <= east_asian_width_N) {
        half_width_count += 1;  // H, Na, N
      } else {
        full_width_count += 1;  // A, W, F
      }
    }
  }
  return full_width_count * 2 + half_width_count;
}

int main(int argc, const char **argv) {
  static char const utf8Str[] = u8"次の決闘がまもなく始まる！";
  int const utf8Strlen = (int)strlen(utf8Str);

  (void)argc;
  (void)argv;

  static_assert(sizeof(utf8Str) == 40, "Basic UTF-8 test");
  init();

  for (int width = 8; width <= 18; width += 5) {
    printf("\n width=%d\n", width);
    printf("v");
    for (int i = 0; i < width; ++i) {
      printf(" ");
    }
    printf("v\n");
    hcbudoux_ctx ctx;
    hcbudoux_init(&ctx, utf8Str, utf8Strlen);

    hcbudoux_span span;
    int x = 0;
    printf(" ");
    while (hcbudoux_getnext_ja(&ctx, &span)) {
      const char *p = utf8Str + span.offset;
      int const n = span.length;
      int const w = computeStringWidth(p, n);
      if (x != 0 && x + w > width) {
        printf("\n ");
        x = 0;
      }
      printf("%.*s", n, p);
      x += w;
    }
    printf("\n");
    // We don't need to "close" hcbudoux_ctx.
  }
}
