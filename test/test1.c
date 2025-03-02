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

static bool test(hcbudoux_impl_lang lang, const void *utf8String, const void *utf8ExpectedResults) {
  int const utf8StringSizeInBytes = (int)strlen((const char *)utf8String);
  const char *pExp = (const char *)utf8ExpectedResults;

  bool total = true;
  hcbudoux_ctx ctx;
  hcbudoux_init(&ctx, utf8String, utf8StringSizeInBytes);
  for (;;) {
    hcbudoux_span span;
    if (!hcbudoux_impl_getnext(&ctx, &span, lang)) {
      break;
    }

    {
      int result = true;
      const char *const expectedStr = pExp;
      int const expectedLen = (int)strlen(expectedStr);
      const char *const actualStr = ((const char *)utf8String) + span.offset;
      int const actualLen = span.length;
      if (expectedLen == 0) {
        result = false;
      }
      if (expectedLen != actualLen) {
        result = false;
      }
      if (result) {
        result = (0 == memcmp(expectedStr, actualStr, actualLen));
      }

      total &= result;

      if (!total) {
        printf("actual   = '%.*s'\n", actualLen, actualStr);
        printf("expected = '%.*s'\n", expectedLen, expectedStr);
      }

      if (*pExp != 0) {
        pExp += expectedLen + 1;
      }
    }
  }
  total &= (0 == *pExp);
  printf("%s", total ? "OK" : "NG");
  printf(": utf8String = [%.*s], len=%d\n", utf8StringSizeInBytes, (const char *)utf8String, utf8StringSizeInBytes);
  return total;
}

static bool test_all(void) {
  typedef struct {
    hcbudoux_impl_lang lang;
    const void *str;
    const void *expected;
  } TestCase;

  static const TestCase testCases[] = {
      // clang-format off
        { hcbudoux_impl_lang_ja,
          u8"私の名前は中野です",
          u8"私の\0"
          u8"名前は\0"
          u8"中野です\0"
        },
        { hcbudoux_impl_lang_ja,
          u8"あなたに寄り添う最先端のテクノロジー",
          u8"あなたに\0"
          u8"寄り添う\0"
          u8"最先端の\0"
          u8"テクノロジー\0"
        },
        { hcbudoux_impl_lang_ja,
          u8"今日は天気です。",
          u8"今日は\0"
          u8"天気です。\0"
        },
        { hcbudoux_impl_lang_ja,
          u8"本日は晴天です。明日は曇りでしょう。",
          u8"本日は\0"
          u8"晴天です。\0"
          u8"明日は\0"
          u8"曇りでしょう。\0"
        },
        { hcbudoux_impl_lang_ja,
          u8"私は遅刻魔で、待ち合わせにいつも遅刻してしまいます。",
          u8"私は\0"
          u8"遅刻魔で、\0"
          u8"待ち合わせに\0"
          u8"いつも\0"
          u8"遅刻してしまいます。\0"
        },
        { hcbudoux_impl_lang_ja,
          u8"メールで待ち合わせ相手に一言、「ごめんね」と謝ればどうにかなると思っていました。",
          u8"メールで\0"
          u8"待ち合わせ相手に\0"
          u8"一言、\0"
          u8"「ごめんね」と\0"
          u8"謝れば\0"
          u8"どうにかなると\0"
          u8"思っていました。\0"
        },
        { hcbudoux_impl_lang_ja,
          u8"海外ではケータイを持っていない。",
          u8"海外では\0"
          u8"ケータイを\0"
          u8"持っていない。\0"
        },
        { hcbudoux_impl_lang_ja, // Test for bad result
          u8"メロスは激怒した。必ず、かの邪智暴虐(じゃちぼうぎゃく)の王を除かなければならぬと決意した。",
          u8"メロスは\0"
          u8"激怒した。\0"
          u8"必ず、\0"
          u8"かの\0"
          u8"邪智暴虐(じゃちぼうぎゃく\0"
          u8")の\0"
          u8"王を\0"
          u8"除かなければなら\0"
          u8"ぬと\0"
          u8"決意した。\0"
        },
        { hcbudoux_impl_lang_ja,
          u8"次の決闘がまもなく始まる！",
          u8"次の\0"
          u8"決闘が\0"
          u8"まもなく\0"
          u8"始まる！\0"
        },

        // vvv Test phrases from https://github.com/google/budoux/blob/v0.6.4/tests/test_parser.py#L109-L164 vvv
        { hcbudoux_impl_lang_ja,
          u8"Google の使命は、世界中の情報を整理し、世界中の人がアクセスできて使えるようにすることです。",
          u8"Google の\0"
          u8"使命は、\0"
          u8"世界中の\0"
          u8"情報を\0"
          u8"整理し、\0"
          u8"世界中の\0"
          u8"人が\0"
          u8"アクセスできて\0"
          u8"使えるように\0"
          u8"する\0"
          u8"ことです。\0"
        },
        { hcbudoux_impl_lang_zh_hans,
          u8"我们的使命是整合全球信息，供大众使用，让人人受益。",
          u8"我们\0"
          u8"的\0"
          u8"使命\0"
          u8"是\0"
          u8"整合\0"
          u8"全球\0"
          u8"信息，\0"
          u8"供\0"
          u8"大众\0"
          u8"使用，\0"
          u8"让\0"
          u8"人\0"
          u8"人\0"
          u8"受益。\0"
        },
        { // Traditional Chinese
          hcbudoux_impl_lang_zh_hant,
          u8"我們的使命是匯整全球資訊，供大眾使用，使人人受惠。",
          u8"我們\0"
          u8"的\0"
          u8"使命\0"
          u8"是\0"
          u8"匯整\0"
          u8"全球\0"
          u8"資訊，\0"
          u8"供\0"
          u8"大眾\0"
          u8"使用，\0"
          u8"使\0"
          u8"人\0"
          u8"人\0"
          u8"受惠。\0"
        },
        // ^^^ Test phrases from https://github.com/google/budoux/blob/v0.6.4/tests/test_parser.py#L109-L164 ^^^
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

int main(int argc, const char **argv) {
  (void)argc;
  (void)argv;
  init();
  return test_all() ? EXIT_SUCCESS : EXIT_FAILURE;
}
