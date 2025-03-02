How to generate hcbudoux.h
--------------------------

- Linux
  ```sh
  make codegen
  ```
- Windows
  ```bat
  .\codegen\run.bat
  ```


`codegen/hcbudoux.template.h`
-----------------------------

[`codegen/hcbudoux.template.h`](hcbudoux.template.h) is an actual "source code" of `include/hcbudoux.h`.

[`codegen/codegen.cpp`](codegen.cpp) is a code generator.

`codegen` reads `hcbudoux.template.h` as a template source and replaces their template variables with [BudouX models](../third_party/budoux/budoux/models/).

`codegen` converts BudouX model JSON files to template variable `BUDOUX_IMPL_TEMPLATE(...)`.  
For example, `third_party/budoux/models/ja.json` is parsed and converted to the follwing variables:
  - UW1 : `HCBUDOUX_IMPL_TEMPLATE(_ja_.UW1)`
  - ...
  - UW6 : `HCBUDOUX_IMPL_TEMPLATE(_ja_.UW6)`
  - BW1 : `HCBUDOUX_IMPL_TEMPLATE(_ja_.BW1)`
  - ...
  - BW3 : `HCBUDOUX_IMPL_TEMPLATE(_ja_.BW3)`
  - TW1 : `HCBUDOUX_IMPL_TEMPLATE(_ja_.TW1)`
  - ...
  - TW4 : `HCBUDOUX_IMPL_TEMPLATE(_ja_.TW4)`
  - base score : `HCBUDOUX_IMPL_TEMPLATE(_ja_.Base)`


Implementation details
----------------------

- Using snake_case, east const, `_impl_` indicates actual (private) implementation.
- Utilize UTF-32 to reduce complexity.
  - `hcbudoux_ctx::utf32s[]` and `indices[]` represents the following relative position characters:
    ```
          +------ index position ('闘')
          |
          v
    次の決闘がま
    ^ ^ ^ ^ ^ ^
    | | | | | |
    | | | | | +-- [5] next + 2 = 'ま'
    | | | | +---- [4] next + 1 = 'が'
    | | | +------ [3] current  = '闘'
    | | +-------- [2] prev - 1 = '決'
    | +---------- [1] prev - 2 = 'の'
    +------------ [0] prev - 3 = '次'
    ```

- `hcbudoux_impl_compute_*` computes a score of `hcbudoux_ctx::utf32s[]` characters.
  - It uses [branchless binary search](https://en.algorithmica.org/hpc/data-structures/binary-search/).
  - For multiple characters key, we encode multiple (up to 3) Unicode codepoints to single `uint64_t`.  Since Unicode is represented in 21 bits, we need 42 bits for 2 codepoints, 63 bits for 3 codepoints.
- See also [BudouX Java implementation](https://github.com/google/budoux/blob/v0.6.4/java/src/main/java/com/google/budoux/Parser.java)
