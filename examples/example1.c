// example1.c - Basic usage
//
// gcc : gcc -std=c11 -I include examples/example1.c && ./a.out
// MSVC: .\script\cl-exe.bat /utf-8 /nologo -I include examples\example1.c && .\example1.exe

#define HCBUDOUX_IMPLEMENTATION 1
#define HCBUDOUX_USE_JA 1  // Use model : ja
#include <assert.h>        // static_assert
#include <stdio.h>         // printf
#include <string.h>        // strlen

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

int main(int argc, const char **argv) {
  static char const utf8Str[] = u8"次の決闘がまもなく始まる！";
  int const utf8Strlen = (int)strlen(utf8Str);

  (void)argc;
  (void)argv;

  static_assert(sizeof(utf8Str) == 40, "Basic UTF-8 test");
  init();

  hcbudoux_ctx ctx;
  hcbudoux_init(&ctx, utf8Str, utf8Strlen);

  hcbudoux_span span;
  while (hcbudoux_getnext_ja(&ctx, &span)) {
    const char *p = utf8Str + span.offset;
    int n = span.length;
    printf("'%.*s'\n", n, p);
  }
  // We don't need to "close" hcbudoux_ctx.
}

// Output:
// '次の'
// '決闘が'
// 'まもなく'
// '始まる！'
