CC = cc
CFLAGS = -std=c99 -Wall -Wextra -O2
BISON = bison
FLEX = flex

all: parser

parser.tab.c: parser.y
	$(BISON) -d -v parser.y

parser.tab.h: parser.tab.c
	@test -f $@ || $(BISON) -d -v parser.y

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

parser: parser.tab.c lex.yy.c ast.c interpreter.c codegen.c main.c ast.h
	$(CC) $(CFLAGS) -o $@ parser.tab.c lex.yy.c ast.c interpreter.c codegen.c main.c

demo: parser
	mkdir -p output
	./parser examples/sum.c output/sum

test: parser
	python3 tests/check.py

clean:
	rm -f parser parser.tab.c parser.tab.h parser.output lex.yy.c

.PHONY: all demo test clean
