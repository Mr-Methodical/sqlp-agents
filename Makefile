CXX := g++
CPPFLAGS := -Isrc
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -g

.PHONY: all run test clean

all: build/sqlp_demo

build/sqlp_demo: src/main.cc src/path.h Makefile
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cc -o $@

build/path_test: tests/path_test.cc src/path.h Makefile
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/path_test.cc -o $@

build/mapping_test: tests/mapping_test.cc src/mapping.cc src/mapping.h Makefile
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/mapping_test.cc src/mapping.cc -o $@

build/sql_test: tests/sql_test.cc src/sql.cc src/sql.h src/mapping.cc \
				src/mapping.h src/path.h src/catalog.cc src/catalog.h Makefile
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/sql_test.cc \
	src/sql.cc src/mapping.cc src/catalog.cc -o $@

build/query_test: tests/query_test.cc src/query.cc src/query.h src/sql.cc \
				src/sql.h src/catalog.cc src/catalog.h src/mapping.cc \
				src/mapping.h src/path.h Makefile
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/query_test.cc src/query.cc \
	src/sql.cc src/catalog.cc src/mapping.cc -o $@

run: build/sqlp_demo
	@./build/sqlp_demo

test: build/path_test build/mapping_test build/sql_test build/query_test
	@./build/path_test
	@./build/mapping_test
	@./build/sql_test
	@./build/query_test

clean:
	rm -rf build
