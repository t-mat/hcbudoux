.DEFAULT_GOAL := run
.PHONY: all clean run
.PHONY: clang-format clang-tidy
.PHONY: codegen examples test

#
all: clean codegen test examples

clean:
	$(MAKE) -C codegen  clean
	$(MAKE) -C examples clean
	$(MAKE) -C test     clean

run: test examples

clang-format:
	$(MAKE) -C codegen  clang-format
	$(MAKE) -C examples clang-format
	$(MAKE) -C test     clang-format

clang-tidy:
	$(MAKE) -C codegen  clang-tidy
	$(MAKE) -C examples clang-tidy
	$(MAKE) -C test     clang-tidy

#
codegen:
	$(MAKE) -C codegen

examples:
	$(MAKE) -C examples

test:
	$(MAKE) -C test
