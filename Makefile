CC?=clang
CFLAGS= -O0 -g -std=c99 -Wall -pedantic -I.

all: all_tests all_toys

all_tests: tests/bin/test_vm tests/bin/test_stack

all_toys: toys/bin/assembler toys/bin/regex_parser toys/bin/compiler toys/bin/charset_parser toys/bin/matcher

tests/bin/test_vm: rex.h tests/src/test_vm.c
	$(CC) tests/src/test_vm.c $(CFLAGS) -o tests/bin/test_vm 

tests/bin/test_stack: rex.h tests/src/test_stack.c
	$(CC) tests/src/test_stack.c $(CFLAGS) -o tests/bin/test_stack 

toys/bin/assembler: rex.h toys/src/assembler.c
	$(CC) toys/src/assembler.c $(CFLAGS) -o toys/bin/assembler

toys/bin/charset_parser: rex.h toys/src/charset_parser.c
	$(CC) toys/src/charset_parser.c $(CFLAGS) -o toys/bin/charset_parser

toys/bin/regex_parser: rex.h toys/src/regex_parser.c
	$(CC) toys/src/regex_parser.c $(CFLAGS) -o toys/bin/regex_parser

toys/bin/compiler: rex.h toys/src/compiler.c
	$(CC) toys/src/compiler.c $(CFLAGS) -o toys/bin/compiler

toys/bin/matcher: rex.h toys/src/matcher.c
	$(CC) toys/src/matcher.c $(CFLAGS) -o toys/bin/matcher


clean:
	rm -f tests/bin/* toys/bin/*
