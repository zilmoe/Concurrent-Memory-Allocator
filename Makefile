CXX = g++
CXXFLAGS = -std=c++20 -I src/ -g -O0 -fsanitize=address -fno-omit-frame-pointer

ALL_SRC_FILES = $(shell find src -type f -name '*.cpp')

SRC_FILES = $(filter-out src/main.cpp, $(ALL_SRC_FILES))

TEST_FILES = $(shell find tst -type f -name '*.cpp')

all:
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(TEST_FILES) $(SRC_FILES) -o bin/test

test: all
	./bin/test

clean:
	rm -rf bin/test
