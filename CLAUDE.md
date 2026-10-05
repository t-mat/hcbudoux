# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

hcbudoux is a single-header C11 implementation of Google's BudouX line break library for Chinese, Japanese, and Thai text. It's a zero-dependency, header-only (stb-style) library with no heap allocation.

## Build Commands

Linux/macOS (make + gcc/clang):

```bash
make            # default target `run`: build and run tests + examples (does NOT regenerate the header)
make all        # clean + codegen + tests + examples
make all-clang  # `make all` with CC=clang CXX=clang++
make codegen    # regenerate include/hcbudoux.h from codegen/ templates + BudouX models
make test       # build and run test/test1 (C) and test/test2 (C++)
make examples   # build and run examples/example1, example2
make clang-format
make clang-tidy
make clean
```

Windows (MSVC, no make):

```bat
.\run.bat                 # default target `run`: tests + examples (does NOT regenerate the header)
.\run.bat all             # clean + codegen + tests + examples
.\run.bat codegen         # regenerate include\hcbudoux.h
.\run.bat test            # tests
.\run.bat examples        # examples
.\run.bat clang-format    # same files and flags as `make clang-format`
.\run.bat clang-tidy      # same files and flags as `make clang-tidy`
.\run.bat clean           # delete _tmp\*.obj and the .exe files
.\run.bat codegen test    # several targets run in order, stopping at the first failure
```

`run.bat` takes the same target names as the Makefile, except `all-clang`. `codegen`, `test`, and `examples` delegate to `codegen\run.bat`, `test\run.bat`, and `examples\run.bat`, which remain usable on their own (like `make -C <dir>`). The `.bat` scripts locate Visual Studio via vswhere (`scripts/cl-exe.bat`, `scripts/intro.bat`), compile with `cl.exe /nologo /utf-8 /O2`, and write `.obj` files to `_tmp/`. They pass `/`-style switches to `cl.exe`, so run them from cmd.exe or PowerShell, not from a Cygwin/MSYS bash. The `clang-format` and `clang-tidy` targets use a project-local LLVM that `scripts/_llvm-ensure.bat` downloads into `.llvm/` (ignored) on first use.

### CI and WSL

`.github/workflows/test.yml` runs `make all` and `make all-clang` on `ubuntu-latest`, then fails if `git diff` shows that the committed `include/hcbudoux.h` differs from the regenerated one. It does not use `run.bat`.

On Windows, the Makefiles run in WSL (`wsl -e make all`). Use `wsl -e`, not `wsl -- bash -lc '...'`: the latter goes through WSL's default shell, which expands `$?` and similar before bash sees them. `scripts/wsl-setup.sh` installs the build tools plus Docker and act (needs sudo); `scripts/act.sh` runs the workflow locally with act. act's image only approximates GitHub's runner.

Releases: the version lives in the `hcbudoux_version_*` enum in `codegen/hcbudoux.template.h` (`scripts/version.sh` prints it). `scripts/release.sh` tags a clean `main` that matches `origin/main` as `v<version>` and pushes the tag; `.github/workflows/release.yml` checks the tag against the header, runs `make all`, and creates the GitHub Release with `include/hcbudoux.h` attached.

## Licensing

The project is CC0-1.0, but the model tables in `include/hcbudoux.h` come from BudouX (Apache-2.0) and `examples/east_asian_width.h` from the Unicode Character Database (Unicode-3.0, notice embedded in the file). Keep both notices when editing those files; the header's notice lives in the template.

### Running a single test binary

There is no per-case selection; the granularity is the binary. `test1` runs every case, prints `OK`/`NG` per case, and exits non-zero if any case fails.

```bash
make -C test test1-run          # C test only
make -C test test2-run          # same tests compiled as C++
make -C examples example1-run
```

## Architecture

### Code Generation Pipeline

`include/hcbudoux.h` is **generated**. Edit `codegen/hcbudoux.template.h`, then run codegen; never edit the output directly.

1. `codegen/codegen.cpp` (C++11, uses `third_party/json.h`) parses each BudouX JSON model in `third_party/budoux/budoux/models/`.
2. Each table entry becomes `{key, score}`. UW keys are a `uint32_t` code point; BW/TW keys pack 2 or 3 code points into a `uint64_t` (21 bits each, first character in the high bits). Entries are emitted sorted by key because codegen stores them in `std::map`. The branchless binary search in the header depends on this ordering.
3. The per-language base score is `-(sum of all scores in the model)`. The header computes `base + 2 * sum(matched)` and breaks when the result is `> 0`, which equals BudouX's `sum(matched) > total / 2` without floating point.
4. `HCBUDOUX_IMPL_TEMPLATE(_ja_.UW1)`-style placeholders in the template are replaced by plain string substitution and written to `include/hcbudoux.h`.

Codegen uses hard-coded relative paths (`../third_party/...`, `./hcbudoux.template.h`, `../include/hcbudoux.h`), so it must run with `codegen/` as the working directory. Both the Makefile and `run.bat` do this.

The template defines `HCBUDOUX_IMPL_TEMPLATE(...)` as an empty macro when it is not already defined, so `codegen/hcbudoux.template.c` (which only includes the template) lets clang-tidy analyze the template as a C translation unit with empty tables.

See `doc/codegen.md` for the sliding-window diagram.

### Header-Only Pattern

Users include the header with `#define HCBUDOUX_IMPLEMENTATION` in exactly one translation unit. Language models are selected at compile time:

```c
#define HCBUDOUX_USE_JA 1       // Japanese
#define HCBUDOUX_USE_JA_KNBC 1  // Japanese (KNBC variant)
#define HCBUDOUX_USE_TH 1       // Thai
#define HCBUDOUX_USE_ZH_HANS 1  // Simplified Chinese
#define HCBUDOUX_USE_ZH_HANT 1  // Traditional Chinese
```

If none are defined, all models are included. `HCBUDOUX_DONT_INCLUDE_STD` skips the `<stdint.h>`/`<stdbool.h>` includes.

### Internal Design

- **UTF-8 input, UTF-32 internal processing**. `hcbudoux_impl_getnext` decodes one code point per iteration into a 6-slot window (`utf32s[3]` is the current character; slots cover prev3..next2), scores it, and returns a span when the score is positive. Reaching the end flushes the final chunk.
- **Branchless binary search** over static sorted tables.
- **Scoring tables**: UW1-UW6 (unigram), BW1-BW3 (bigram), TW1-TW4 (trigram).

### Tests

- `test/test1.c` defines `HCBUDOUX_IMPLEMENTATION` and calls the **private** `hcbudoux_impl_getnext` with `hcbudoux_impl_lang_*`, so renaming internals breaks the tests.
- An expected result is one string with the segments joined by `▁` (U+2581), e.g. `u8"次の▁決闘が▁まもなく▁始まる！"`. `test()` joins the spans it gets from the parser with the same separator and compares the whole string once; a case whose input contains the separator fails. Add cases to `testCases[]` in `test_all()`.
- `test/test2.cpp` is just `#include "./test1.c"` compiled as C++ (`-std=c++11`, MSVC `/std:c++14`). Anything added to `test1.c` must also be valid C++ (no implicit `void*` conversions, etc.).
- Some Japanese expectations (marked "Test for bad result") are model-specific and change when the BudouX model changes.

### Key Files

- `codegen/hcbudoux.template.h` — Template for the public API and implementation
- `codegen/hcbudoux.template.c` — Wrapper for clang-tidy to analyze the template as C
- `codegen/codegen.cpp` — Code generator (parses JSON models, produces C arrays)
- `include/hcbudoux.h` — Generated output (do not edit directly)
- `test/test1.c` — Main C test suite; `test/test2.cpp` — C++ compatibility test
- `examples/example1.c` — Basic usage; `examples/example2.c` — Auto line-breaking with width
- `examples/east_asian_width.h` — Display-width table converted from Unicode EastAsianWidth.txt; used only by example2

## Updating third-party dependencies

BudouX and json.h are vendored and pinned by commit hash in `third_party/budoux_hash.txt` and `third_party/json_h_hash.txt`. Procedure (see commits "Bumped budoux to vX.Y.Z"):

1. Put the new commit hash in the hash file.
2. Run `third_party/download.sh` (or `download.bat`). It re-fetches only the model JSONs, `json.h`, and LICENSE files.
3. Run codegen, then the tests. Fix expectations in `test/test1.c` if a model changed.
4. Update the BudouX version row in the README table.

## Compiler Flags

C11 with strict warnings: `-std=c11 -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow -Wswitch-enum -Wstrict-prototypes -Wundef -Wpointer-arith -Wstrict-aliasing=1`

C++ sources (`codegen.cpp`, `test2.cpp`) use `-std=c++11` with the same set minus `-Wstrict-prototypes`. MSVC uses `/std:c11` and `/std:c++14 /EHsc`.

## Code Style

Google C style, 120-character column limit (see `.clang-format`). Qualifiers are west const (`const T *p`, `static const T x[]`), enforced by `QualifierAlignment: Left`; pointers are right-aligned (`T *p`). `make clang-format` formats the template, codegen, tests, and examples only, never `include/hcbudoux.h` or `examples/east_asian_width.h`. The test table in `test1.c` is wrapped in `// clang-format off`.

`.clang-tidy` at the root lists the check set explicitly (LLVM 23's default is empty and errors out) and disables `clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling`, which in C11 mode flags every `memcpy`/`snprintf` in favour of the optional Annex K `*_s` functions that glibc does not provide. Keep analyzer exclusions there, not as `NOLINT` comments in code. Note that the analyzer counts embedded NULs in a string literal as characters when it models `strlen()` (LLVM 23 treats `strlen("ab\0cd")` as 5), so NUL-separated lists in test data would trigger `clang-analyzer-security.ArrayBound`.

## Repository Gotchas

- `.gitignore` files are **allow-lists** (`/*` followed by `!` entries). A new top-level directory, or a file inside `codegen/`, `test/`, or `examples/` with an extension other than `.c/.cpp/.h/.md/.sh/.bat` (plus `Makefile`), is silently ignored until the corresponding `.gitignore` is updated.
- `.gitattributes` forces LF for `Makefile` and `*.sh`, CRLF for `*.bat`.
- `_tmp/` holds MSVC objects and download staging; it is ignored.

## Design Decisions

- Public API uses `int32_t` for sizes and offsets. Negative size is treated as 0.
- Invalid UTF-8 bytes are advanced 1 byte at a time (no validation, raw bytes passed through).
- UTF-8 decoding logic is intentionally duplicated across `hcbudoux.template.h`, `codegen.cpp`, and `example2.c`. Each file is independent.
- `readFile()` in codegen returns empty string on failure by design; `generate()` detects this via empty template map.
- Thai test phrases are absent because upstream BudouX has none.
