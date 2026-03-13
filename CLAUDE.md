# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

hcbudoux is a single-header C11 implementation of Google's BudouX line break library for Chinese, Japanese, and Thai text. It's a zero-dependency, header-only (stb-style) library with no heap allocation.

## Build Commands

```bash
# Build everything (codegen + tests + examples)
make all

# Run tests only
make test

# Run code generation (regenerates include/hcbudoux.h from templates + BudouX models)
make codegen

# Build and run examples
make examples

# Format code
make clang-format

# Run static analysis
make clang-tidy

# Clean build artifacts
make clean
```

On Windows, use `run.bat` for tests/examples, and `codegen/run.bat` for code generation.

## Architecture

### Code Generation Pipeline

The main header `include/hcbudoux.h` is **generated**, not hand-edited. The pipeline:

1. `codegen/codegen.cpp` parses BudouX JSON models from `third_party/budoux/budoux/models/`
2. Converts model data into C arrays (optimized lookup tables)
3. Substitutes placeholders in `codegen/hcbudoux.template.h`
4. Outputs the final `include/hcbudoux.h`

**To modify the library API or implementation, edit the templates in `codegen/`, not `include/hcbudoux.h` directly.**

### Header-Only Pattern

Users include the header with `#define HCBUDOUX_IMPLEMENTATION` in exactly one translation unit. Language models are selected at compile time:

```c
#define HCBUDOUX_USE_JA 1       // Japanese
#define HCBUDOUX_USE_JA_KNBC 1  // Japanese (KNBC variant)
#define HCBUDOUX_USE_TH 1       // Thai
#define HCBUDOUX_USE_ZH_HANS 1  // Simplified Chinese
#define HCBUDOUX_USE_ZH_HANT 1  // Traditional Chinese
```

If none are defined, all models are included.

### Internal Design

- **UTF-8 input, UTF-32 internal processing** — 6-character sliding window for scoring
- **Branchless binary search** over static lookup tables for performance
- **Multi-character keys** pack UTF-32 codepoints (21 bits each) into uint64_t for bigram/trigram lookups
- **Scoring tables**: UW1-UW6 (unigram), BW1-BW3 (bigram), TW1-TW4 (trigram)

### Key Files

- `codegen/hcbudoux.template.h` — Template for the public API and implementation
- `codegen/hcbudoux.template.c` — Wrapper for clang-tidy to analyze the template as C
- `codegen/codegen.cpp` — Code generator (parses JSON models, produces C arrays)
- `include/hcbudoux.h` — Generated output (do not edit directly)
- `test/test1.c` — Main C test suite; `test/test2.cpp` — C++ compatibility test
- `examples/example1.c` — Basic usage; `examples/example2.c` — Auto line-breaking with width

## Compiler Flags

C11 with strict warnings: `-std=c11 -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow -Wswitch-enum -Wstrict-prototypes -Wundef -Wpointer-arith -Wstrict-aliasing=1`

## Code Style

Google C style, 120-character column limit (see `.clang-format`).

## Design Decisions

- Public API uses `int32_t` for sizes and offsets. Negative size is treated as 0.
- Invalid UTF-8 bytes are advanced 1 byte at a time (no validation, raw bytes passed through).
- UTF-8 decoding logic is intentionally duplicated across `hcbudoux.template.h`, `codegen.cpp`, and `example2.c` — each file is independent.
- `readFile()` in codegen returns empty string on failure by design; `generate()` detects this via empty template map.
- Thai test phrases are absent because upstream BudouX has none.
