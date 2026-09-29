// test1 - basic test
#define HCBUDOUX_IMPLEMENTATION 1
#include <stdbool.h>  // bool, true, false
#include <stdint.h>   // uint8_t, uint32_t, uint64_t
#include <stdio.h>    // printf
#include <stdlib.h>   // EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>   // strlen

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

// Segments of an expected result are joined with a separator, e.g. u8"私の▁名前は▁中野です".
// test() joins the spans returned by hcbudoux with the same separator and compares the whole string
// once, so an expected value is an ordinary NUL-terminated string literal.
static bool test(hcbudoux_impl_lang lang, const char *utf8String, const char *expected) {
  static const char separator[] = u8"▁";  // U+2581 LOWER ONE EIGHTH BLOCK, the boundary marker of BudouX
  const int32_t utf8StringSizeInBytes = (int32_t)strlen(utf8String);
  char actual[512] = {0};
  size_t actualLen = 0;

  if (strstr(utf8String, separator) != NULL) {
    printf("NG: utf8String = [%s] contains the separator %s\n", utf8String, separator);
    return false;
  }

  hcbudoux_ctx ctx;
  hcbudoux_init(&ctx, utf8String, utf8StringSizeInBytes);
  for (;;) {
    hcbudoux_span span;
    if (!hcbudoux_impl_getnext(&ctx, &span, lang)) {
      break;
    }
    // Append "<separator><span>" to actual, bounded by the remaining buffer size.
    const size_t room = sizeof(actual) - actualLen;
    const int n = snprintf(actual + actualLen, room, "%s%.*s", (actualLen != 0) ? separator : "", (int)span.length,
                           utf8String + span.offset);
    if (n < 0 || (size_t)n >= room) {
      printf("NG: utf8String = [%s] does not fit in the actual buffer\n", utf8String);
      return false;
    }
    actualLen += (size_t)n;
  }

  const bool ok = (strcmp(actual, expected) == 0);
  printf("%s: utf8String = [%s], len=%d\n", ok ? "OK" : "NG", utf8String, utf8StringSizeInBytes);
  if (!ok) {
    printf("  actual   = [%s]\n", actual);
    printf("  expected = [%s]\n", expected);
  }
  return ok;
}

static bool test_all(void) {
  typedef struct {
    hcbudoux_impl_lang lang;
    const char *str;
    const char *expected;
  } TestCase;

  static const TestCase testCases[] = {
      // clang-format off
        // hcbudoux_impl_lang_ja
        { hcbudoux_impl_lang_ja,
          u8"私の名前は中野です",
          u8"私の▁名前は▁中野です"
        },
        { hcbudoux_impl_lang_ja,
          u8"あなたに寄り添う最先端のテクノロジー",
          u8"あなたに▁寄り添う▁最先端の▁テクノロジー"
        },
        { hcbudoux_impl_lang_ja,
          u8"今日は天気です。",
          u8"今日は▁天気です。"
        },
        { hcbudoux_impl_lang_ja,
          u8"本日は晴天です。明日は曇りでしょう。",
          u8"本日は▁晴天です。▁明日は▁曇りでしょう。"
        },
        { hcbudoux_impl_lang_ja,
          u8"私は遅刻魔で、待ち合わせにいつも遅刻してしまいます。",
          u8"私は▁遅刻魔で、▁待ち合わせに▁いつも▁遅刻してしまいます。"
        },
        { hcbudoux_impl_lang_ja,
          u8"メールで待ち合わせ相手に一言、「ごめんね」と謝ればどうにかなると思っていました。",
          u8"メールで▁待ち合わせ相手に▁一言、▁「ごめんね」と▁謝れば▁どうにかなると▁思っていました。"
        },
        { hcbudoux_impl_lang_ja,
          u8"海外ではケータイを持っていない。",
          u8"海外では▁ケータイを▁持っていない。"
        },
        { hcbudoux_impl_lang_ja, // Test for bad result
          u8"メロスは激怒した。必ず、かの邪智暴虐(じゃちぼうぎゃく)の王を除かなければならぬと決意した。",
          u8"メロスは▁激怒した。▁必ず、▁かの▁邪智暴虐(じゃちぼうぎゃく▁)の▁王を▁除かなければならぬと▁決意した。"
        },
        { hcbudoux_impl_lang_ja,
          u8"次の決闘がまもなく始まる！",
          u8"次の▁決闘が▁まもなく▁始まる！"
        },

        // hcbudoux_impl_lang_ja_knbc
        { hcbudoux_impl_lang_ja_knbc,
          u8"私の名前は中野です",
          u8"私の▁名前は▁中野です"
        },
        { hcbudoux_impl_lang_ja_knbc,
          u8"あなたに寄り添う最先端のテクノロジー",
          u8"あなたに▁寄り添う▁最先端の▁テクノロジー"
        },
        { hcbudoux_impl_lang_ja_knbc,
          u8"今日は天気です。",
          u8"今日は▁天気です。"
        },
        { hcbudoux_impl_lang_ja_knbc,
          u8"本日は晴天です。明日は曇りでしょう。",
          u8"本日は▁晴天です。▁明日は▁曇りでしょう。"
        },
        { hcbudoux_impl_lang_ja_knbc,
          u8"私は遅刻魔で、待ち合わせにいつも遅刻してしまいます。",
          u8"私は▁遅刻魔で、▁待ち合わせに▁いつも▁遅刻してしまいます。"
        },
        { hcbudoux_impl_lang_ja_knbc,
          u8"メールで待ち合わせ相手に一言、「ごめんね」と謝ればどうにかなると思っていました。",
          u8"メールで▁待ち合わせ相手に▁一言、▁「ごめんね」と▁謝れば▁どうにかなると▁思っていました。"
        },
        { hcbudoux_impl_lang_ja_knbc,
          u8"海外ではケータイを持っていない。",
          u8"海外では▁ケータイを▁持っていない。"
        },
        { hcbudoux_impl_lang_ja_knbc, // Test for bad result
          u8"メロスは激怒した。必ず、かの邪智暴虐(じゃちぼうぎゃく)の王を除かなければならぬと決意した。",
          u8"メロスは▁激怒した。▁必ず、▁かの▁邪智暴虐(じゃちぼうぎゃく▁)の▁王を▁除かなければなら▁ぬと▁決意した。"
        },
        { hcbudoux_impl_lang_ja_knbc,
          u8"次の決闘がまもなく始まる！",
          u8"次の▁決闘が▁まも▁なく▁始まる！"
        },

        // vvv Test phrases from https://github.com/google/budoux/blob/v0.7.0/tests/test_parser.py#L109-L164 vvv
        { hcbudoux_impl_lang_ja,
          u8"Google の使命は、世界中の情報を整理し、世界中の人がアクセスできて使えるようにすることです。",
          u8"Google の▁使命は、▁世界中の▁情報を▁整理し、▁世界中の▁人が▁アクセスできて▁使えるように▁する▁ことです。"
        },
        { hcbudoux_impl_lang_zh_hans,
          u8"我们的使命是整合全球信息，供大众使用，让人人受益。",
          u8"我们▁的▁使命▁是▁整合▁全球▁信息，▁供▁大众▁使用，▁让▁人▁人▁受益。"
        },
        { // Traditional Chinese
          hcbudoux_impl_lang_zh_hant,
          u8"我們的使命是匯整全球資訊，供大眾使用，使人人受惠。",
          u8"我們▁的▁使命▁是▁匯整▁全球▁資訊，▁供▁大眾▁使用，▁使▁人▁人▁受惠。"
        },
        // ^^^ Test phrases from https://github.com/google/budoux/blob/v0.7.0/tests/test_parser.py#L109-L164 ^^^
        // TODO: There's no Thai test phrase.  We must have it.
      // clang-format on
  };

  bool result = true;
  for (int i = 0; i < (int)(sizeof(testCases) / sizeof(testCases[0])); ++i) {
    const TestCase *const testCase = &testCases[i];
    result &= test(testCase->lang, testCase->str, testCase->expected);
  }
  return result;
}

// Edge case tests
static bool test_edge_cases(void) {
  bool result = true;

  // Empty string
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    hcbudoux_init(&ctx, u8"", 0);
    bool got = hcbudoux_impl_getnext(&ctx, &span, hcbudoux_impl_lang_ja);
    bool ok = !got;
    printf("%s: empty string\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // Single ASCII character
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    hcbudoux_init(&ctx, u8"A", 1);
    bool got = hcbudoux_getnext_ja(&ctx, &span);
    bool ok = got && span.offset == 0 && span.length == 1;
    if (got) {
      got = hcbudoux_getnext_ja(&ctx, &span);
      ok &= !got;
    }
    printf("%s: single ASCII char\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // Single CJK character
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    hcbudoux_init(&ctx, u8"漢", 3);
    bool got = hcbudoux_getnext_ja(&ctx, &span);
    bool ok = got && span.offset == 0 && span.length == 3;
    if (got) {
      got = hcbudoux_getnext_ja(&ctx, &span);
      ok &= !got;
    }
    printf("%s: single CJK char\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // ASCII-only string
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    hcbudoux_init(&ctx, u8"Hello World", 11);
    bool got = hcbudoux_getnext_ja(&ctx, &span);
    bool ok = got && span.offset == 0 && span.length == 11;
    if (got) {
      got = hcbudoux_getnext_ja(&ctx, &span);
      ok &= !got;
    }
    printf("%s: ASCII-only string\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // 4-byte UTF-8 character (emoji)
  {
    static const char str[] = u8"笑顔\xF0\x9F\x98\x80です";  // U+1F600
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    int32_t total_len = 0;
    hcbudoux_init(&ctx, str, (int32_t)strlen(str));
    while (hcbudoux_getnext_ja(&ctx, &span)) {
      total_len += span.length;
    }
    bool ok = total_len == (int32_t)strlen(str);
    printf("%s: 4-byte UTF-8 (emoji)\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // Negative size treated as 0
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    hcbudoux_init(&ctx, u8"test", -1);
    bool got = hcbudoux_getnext_ja(&ctx, &span);
    bool ok = !got;
    printf("%s: negative size\n", ok ? "OK" : "NG");
    result &= ok;
  }

  return result;
}

// Public API tests
static bool test_public_api(void) {
  bool result = true;

  // Test hcbudoux_getnext_ja via public API
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    static const char str[] = u8"次の決闘がまもなく始まる！";
    result &= test(hcbudoux_impl_lang_ja, str, u8"次の▁決闘が▁まもなく▁始まる！");
    // Also verify via public API function
    hcbudoux_init(&ctx, str, (int32_t)strlen(str));
    bool got = hcbudoux_getnext_ja(&ctx, &span);
    bool ok = got && span.offset == 0 && span.length == 6;  // "次の" = 6 bytes
    printf("%s: public API hcbudoux_getnext_ja\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // Test hcbudoux_getnext_zh_hans via public API
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    static const char str[] = u8"我们的使命";
    hcbudoux_init(&ctx, str, (int32_t)strlen(str));
    bool got = hcbudoux_getnext_zh_hans(&ctx, &span);
    bool ok = got && span.offset == 0 && span.length > 0;
    int32_t total_len = 0;
    hcbudoux_init(&ctx, str, (int32_t)strlen(str));
    while (hcbudoux_getnext_zh_hans(&ctx, &span)) {
      total_len += span.length;
    }
    ok &= total_len == (int32_t)strlen(str);
    printf("%s: public API hcbudoux_getnext_zh_hans\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // Test hcbudoux_getnext_zh_hant via public API
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    static const char str[] = u8"我們的使命";
    int32_t total_len = 0;
    hcbudoux_init(&ctx, str, (int32_t)strlen(str));
    while (hcbudoux_getnext_zh_hant(&ctx, &span)) {
      total_len += span.length;
    }
    bool ok = total_len == (int32_t)strlen(str);
    printf("%s: public API hcbudoux_getnext_zh_hant\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // Test hcbudoux_getnext_th via public API
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    static const char str[] = u8"ภารกิจของเรา";
    int32_t total_len = 0;
    hcbudoux_init(&ctx, str, (int32_t)strlen(str));
    while (hcbudoux_getnext_th(&ctx, &span)) {
      total_len += span.length;
    }
    bool ok = total_len == (int32_t)strlen(str);
    printf("%s: public API hcbudoux_getnext_th\n", ok ? "OK" : "NG");
    result &= ok;
  }

  // Test hcbudoux_getnext_ja_knbc via public API
  {
    hcbudoux_ctx ctx;
    hcbudoux_span span;
    static const char str[] = u8"次の決闘がまもなく始まる！";
    int32_t total_len = 0;
    hcbudoux_init(&ctx, str, (int32_t)strlen(str));
    while (hcbudoux_getnext_ja_knbc(&ctx, &span)) {
      total_len += span.length;
    }
    bool ok = total_len == (int32_t)strlen(str);
    printf("%s: public API hcbudoux_getnext_ja_knbc\n", ok ? "OK" : "NG");
    result &= ok;
  }

  return result;
}

int main(int argc, const char **argv) {
  (void)argc;
  (void)argv;
  init();
  bool result = true;
  result &= test_all();
  result &= test_edge_cases();
  result &= test_public_api();
  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
