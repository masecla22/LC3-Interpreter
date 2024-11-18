.PHONY: all test clean lexer parser string_map cli
.DEFAULT_GOAL := all
.SILENT: test


CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -g


test: all
		echo "\n"
		valgrind ./target/lc3 --input=samples/ata/bf.asm --output==-

all: parser lexer string_map cli
		mkdir -p target
		$(CC) $(CFLAGS) -o target/main.o -c src/main.c
		$(CC) $(CFLAGS) -o target/lc3 target/main.o target/lexer/lexer.o target/grammar/parser.o target/map/string_map.o target/cli/cli.o target/cli/default/default_cli.o -lfl -lm

lexer: src/lexer/lexer.fl
		mkdir -p target/lexer
		flex -o target/lexer/lexer.c src/lexer/lexer.fl 
		$(CC) $(CFLAGS) -Wno-unused-function -c target/lexer/lexer.c -o target/lexer/lexer.o

parser: src/grammar/parser.y
		 mkdir -p target/grammar
		 bison -d -o target/grammar/parser.c src/grammar/parser.y 
		 $(CC) $(CFLAGS) -O2 -c target/grammar/parser.c -o target/grammar/parser.o

string_map: src/map/string_map.c
		 mkdir -p target/map
		 $(CC) $(CFLAGS) -c src/map/string_map.c -o target/map/string_map.o

cli: src/cli/cli.c src/cli/default/default_cli.c
		 mkdir -p target/cli
		 mkdir -p target/cli/default
		 $(CC) $(CFLAGS) -c src/cli/cli.c -o target/cli/cli.o
		 $(CC) $(CFLAGS) -c src/cli/default/default_cli.c -o target/cli/default/default_cli.o

clean:
	rm -rf target