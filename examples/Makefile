.DEFAULT_GOAL := run
.PHONY: clean run
.PHONY: clang-format clang-tidy
.PHONY: example1 example1-run
.PHONY: example2 example2-run

CFLAGS ?= -I../include -std=c11 \
          -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow \
          -Wswitch-enum -Wstrict-prototypes \
          -Wundef -Wpointer-arith -Wstrict-aliasing=1

clean:
	rm -f ./*.o example1 example2

clang-format:
	clang-format -i example1.c
	clang-format -i example2.c

clang-tidy:
	clang-tidy example1.c -- $(CFLAGS)
	clang-tidy example2.c -- $(CFLAGS)

run: example1-run example2-run

example1: example1.o
	$(CC) -o $@ $^ $(CFLAGS)

example1-run: example1
	./$^

example2: example2.o
	$(CC) -o $@ $^ $(CFLAGS)

example2-run: example2
	./$^

$(ODIR)/%.o: %.c ../include/hcbudoux.h
	$(CC) -c -o $@ $< $(CFLAGS)
