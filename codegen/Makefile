.DEFAULT_GOAL := run
.PHONY: clean run
.PHONY: clang-format clang-tidy
.PHONY: codegen codegen-run

CFLAGS ?= -I../include -std=c11 \
          -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow \
          -Wswitch-enum -Wstrict-prototypes \
          -Wundef -Wpointer-arith -Wstrict-aliasing=1

CXXFLAGS ?= -I../include -I../third_party/json.h -std=c++11 \
            -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wshadow \
            -Wswitch-enum -Wundef -Wpointer-arith -Wstrict-aliasing=1

clean:
	rm -f ./*.o codegen

clang-format:
	clang-format -i codegen.cpp
	clang-format -i hcbudoux.template.h

clang-tidy:
	clang-tidy codegen.cpp -header-filter= -- $(CXXFLAGS)
	clang-tidy hcbudoux.template.c -- $(CFLAGS)

run: codegen-run

codegen: codegen.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS)

codegen-run: codegen
	./$^

$(ODIR)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)
