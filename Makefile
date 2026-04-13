

# Build and test automation for the project

CC := gcc
CFLAGS := -Wall -Wextra -pedantic

MAIN_TARGET := lz78_main
TEST_TARGET := lz78_test

.PHONY: all build test test-c clean help

all: clean build test

build: $(MAIN_TARGET) $(TEST_TARGET)

$(MAIN_TARGET): main.o lz78.o
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_TARGET): test.o lz78.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c lz78.h
	$(CC) $(CFLAGS) -c main.c -o $@

test.o: test.c lz78.h
	$(CC) $(CFLAGS) -c test.c -o $@

lz78.o: lz78.c lz78.h
	$(CC) $(CFLAGS) -c lz78.c -o $@

test: test-c

test-c: $(TEST_TARGET)
	./$(TEST_TARGET)


clean:
	rm -f *.o $(MAIN_TARGET) $(TEST_TARGET)
	rm -rf tests/artifacts
	rm -f tests/images/*.lz78 tests/images/*.roundtrip.*
	rm -rf tests/images/compare_outputs
	rm -f restored.jpeg
