hcbudoux
========

A single-header C11 implementation of [BudouX](https://github.com/google/budoux) line break library for Chinese, Japanese, and Thai languages.


Prerequisites
-------------

- C11 compatible C/C++ compiler (clang, gcc, MSVC)


Install and usage
-----------------

- Copy `include/hcbudoux.h` to your project.
- Follow the stb-style header-only library convention. Therefore, you should do this:
  ```C
  #define HCBUDOUX_IMPLEMENTATION
  #include "hcbudoux.h"
  ```
  in *one* C or C++ file to create the implementation.


Example
-------

```C
// gcc : gcc -std=c11 -I include examples/example1.c && ./a.out
// MSVC: .\script\cl-exe.bat /utf-8 /nologo -I include examples\example1.c && .\example1.exe
#define HCBUDOUX_IMPLEMENTATION 1
#define HCBUDOUX_USE_JA 1 // Use model : ja
#include "hcbudoux.h"
#include <assert.h> // static_assert
#include <stdio.h>  // printf
#include <string.h> // strlen

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
void init(void) { SetConsoleOutputCP(CP_UTF8); }
#else
void init(void) {}
#endif

int main(int argc, const char **argv) {
    static char const utf8_str[]  = u8"次の決闘がまもなく始まる！";
    int const         utf8_strlen = (int)strlen(utf8_str);
    init();

    hcbudoux_ctx ctx;
    hcbudoux_init(&ctx, utf8_str, utf8_strlen);
    static_assert(sizeof(utf8_str) == 40, "Basic UTF-8 test");

    hcbudoux_span span;
    while (hcbudoux_getnext_ja(&ctx, &span)) {
        const char *p = utf8_str + span.offset;
        int         n = span.length;
        printf("'%.*s'\n", n, p);
    }
    // We don't need to "close" hcbudoux_ctx.
}
```

Output:

```
'次の'
'決闘が'
'まもなく'
'始まる！'
```


BudouX and "The next duel is about to beg＜br＞in!" problem
-----------------------------------------------------------

Here's an example of a Japanese text from a video game 
["Kingdom Come: Deliverance II" (2025)](https://en.wikipedia.org/wiki/Kingdom_Come%3A_Deliverance_II):

!["The next duel is about to beg<br>in!"](doc/media/nextduel.png)

This message displays "次の決闘がまもなく始まる！" in Japanese,
which means "The next duel is about to begin!" in English.
The problem is a line break of the message:

```
# NG
次の決闘がまもなく始ま  
る！
```

Since "始まる" is a single (unsplittable) Japanese word that means "(to) begin,"
this line break is unnatural. In English, it may look like:

```
# NG
The next duel is about to beg  
in!
```

As you see, it is not a trivial task to add a line break properly.
Because Japanese text doesn't have explicit word separators/spaces.
To solve (ease) this issue, BudouX gives clues for word separation.
See also: [BudouX - Background](https://github.com/google/budoux?tab=readme-ov-file#background)

Note that BudouX models are not perfect.  They may fail to estimate a line break position.

For example, `test/test1.c` contains a "bad result" in Japanese `除かなければならぬと決意した。`.

Actual output of BudouX is

```
# NG
除かなければなら  
ぬと  
決意した。
```

But `ならぬ` is a single word.  So it should be

```
# OK
除かなければ  
ならぬと  
決意した。
```


Public API
----------

```C
struct hcbudoux_ctx;   // Parser context. (All members are private)
struct hcbudoux_span { // String view
  int offset;          // Public member: offset in bytes.
  int length;          // Public member: length in bytes.
};

// Initialize a parser context with a UTF-8 string.
// utf8_str is encoded in UTF-8.  The lifetime of utf8_str is longer than hcbudoux_ctx.
// We don't need to "close" hcbudoux_ctx since it doesn't allocate dynamic resources.
void hcbudoux_init (hcbudoux_ctx *ctx, const void *utf8_str, int utf8_str_size_in_bytes);

// Get the next string view of the specific language.
// Returns false when the parser reaches the end of utf8_str.
// When it returns true, span contains a valid offset and length of the string view.
bool hcbudoux_getnext_ja      (hcbudoux_ctx *ctx, hcbudoux_span* span);
bool hcbudoux_getnext_ja_knbc (hcbudoux_ctx *ctx, hcbudoux_span* span);
bool hcbudoux_getnext_th      (hcbudoux_ctx *ctx, hcbudoux_span* span);
bool hcbudoux_getnext_zh_hans (hcbudoux_ctx *ctx, hcbudoux_span* span);
bool hcbudoux_getnext_zh_hant (hcbudoux_ctx *ctx, hcbudoux_span* span);
```


Details
-------

There are compile-time configuration macros.  You can specify which model(s) you will use.
If all symbols are not defined, `hcbudoux.h` includes all available models.

```C
#define HCBUDOUX_IMPLEMENTATION 1
#define HCBUDOUX_USE_JA      1 // Use model : ja
#define HCBUDOUX_USE_JA_KNBC 1 // Use model : ja_knbc
#define HCBUDOUX_USE_TH      1 // Use model : th
#define HCBUDOUX_USE_ZH_HANS 1 // Use model : zh-hans
#define HCBUDOUX_USE_ZH_HANT 1 // Use model : zh-hant
#include "hcbudoux.h"
```

hcbudoux uses the following BudouX models, C standards, headers, types and constants:

| -                                     | -                                             |
| ---                                   | ---                                           |
| C language standard                   | C11                                           |
| Prefix of C-language symbols          | `hcbudoux_`                                   |
| Prefix of C-preprocessor symbols      | `HCBUDOUX_`                                   |
| BudouX version                        | `v0.6.4`, [1f20187](https://github.com/google/budoux/commit/1f201873ccaf38cd318a2c4f07ae9f8b88a1f315) |
| BudouX natural language models        | ja, ja_knbc, th, zh-hans, zh-hant             |
| Standard header dependencies          | `<stdint.h>` <br> `<stdbool.h>`               |
| `<stdint.h>` types in use             | `uint8_t`, `uint32_t`, `uint64_t`             |
| `<stdbool.h>` types in use            | `bool`, `true`, `false`                       |
| Standard library binary dependencies  |  No dependency.                               |
| External resource allocation          |  No heap memory allocation, I/O, callback.    |

[doc/codegen.md](doc/codegen.md) describes the details of `codegen` and `hcbudoux.h`.


Shortcut for development tasks
------------------------------

| Task                                  | Linux                 | Windows                               |
| ---                                   | ---                   | ---                                   |
| Run codegen, test and examples        | `make all`            | `.\codegen\run.bat && .\run.bat`      |
| Run codegen                           | `make codegen`        | `.\codegen\run.bat`                   |
| Run test                              | `make test`           | `.\test\run.bat`                      |
| Run examples                          | `make examples`       | `.\examples\run.bat`                  |
| Run clang-format                      | `make clang-format`   | -                                     |
| Run clang-tidy                        | `make clang-tidy`     | -                                     |


Third party libraries
---------------------

hcbudoux uses the following third party libraries:

- [google/BudouX](https://github.com/google/budoux)
- [sheredom/json.h](https://github.com/sheredom/json.h)

The following files contain the target commit hash of each library.

| Third party library   | Hash file                     |
| ---                   | ---                           |
| BudouX                | `third_party/budoux_hash.txt` |
| json.h                | `third_party/json_h_hash.txt` |

You can download them with the following scripts

| Task                                  | Linux                         | Windows                       |
| ---                                   | ---                           | ---                           |
| Download third party libraries        | `./third_party/download.sh`   | `.\third_party\download.bat`  |


Alternatives
------------

There're alternative line break libraries:
[BudouX](https://github.com/google/budoux/),
[TinySegmenter](http://www.chasen.org/~taku/software/TinySegmenter/),
[TinySegmenterMaker](https://github.com/shogo82148/TinySegmenterMaker/),
[zig-budoux](https://github.com/Cloudef/zig-budoux)


License
-------

```
"hcbudoux" by Takayuki Matsuoka.

To the extent possible under law, the person who associated CC0-1.0 with "hcbudoux".
See https://creativecommons.org/publicdomain/zero/1.0/ for CC0-1.0 legalcode.
```
