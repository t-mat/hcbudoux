.DEFAULT_GOAL := run
.PHONY: clean run
.PHONY: clang-format clang-tidy
.PHONY: test1 test1-run
.PHONY: test2 test2-run

CFLAGS ?= -I../include -std=c11 \
          -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow \
          -Wswitch-enum -Wstrict-prototypes \
          -Wundef -Wpointer-arith -Wstrict-aliasing=1

CXXFLAGS ?= -I../include -std=c++11 \
            -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow \
            -Wswitch-enum -Wundef -Wpointer-arith -Wstrict-aliasing=1

clean:
	rm -f ./*.o test1 test2

clang-format:
	clang-format -i test1.c

clang-tidy:
	clang-tidy test1.c -- $(CFLAGS)

run: test1-run test2-run

test1-run: test1
	./$^

test2-run: test2
	./$^

test1: test1.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test2: test2.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(ODIR)/%.o: %.c ../include/hcbudoux.h
	$(CC) -c -o $@ $< $(CFLAGS)

