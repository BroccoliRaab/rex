CC?=clang
CFLAGS= -O0 -g -std=c99 -Wall -pedantic -I.

all: all_tests all_toys

all_tests: tests/bin/test_vm

all_toys: toys/bin/assembler toys/bin/charset_parser

tests/bin/test_vm: rex.h tests/src/test_vm.c
	$(CC) tests/src/test_vm.c $(CFLAGS) -o tests/bin/test_vm 

toys/bin/assembler: rex.h toys/src/assembler.c
	$(CC) toys/src/assembler.c $(CFLAGS) -o toys/bin/assembler

toys/bin/charset_parser: rex.h toys/src/charset_parser.c
	$(CC) toys/src/charset_parser.c $(CFLAGS) -o toys/bin/charset_parser


clean:
	rm -f tests/bin/* toys/bin/*
