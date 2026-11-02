CXX ?= g++
CPPFLAGS := -I.
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
HEADERS := $(wildcard *.h)

.PHONY: all run test clean
all: academia

academia: main.cpp $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) main.cpp -o $@

run: academia
	./academia

build/testes: tests/testes.cpp $(HEADERS)
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/testes.cpp -o $@

test: academia build/testes
	./build/testes

clean:
	rm -f academia
	rm -rf build
