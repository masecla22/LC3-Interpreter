.PHONY: all test clean lexer parser string_map cli
.DEFAULT_GOAL := all
.SILENT: test all lexer parser string_map cli clean lc3


CC = gcc
CFLAGS = -Wno-unused-result -Wno-unused-parameter -Wall -Wextra -Werror -pedantic -g -O2


test: all
		./target/lc3 --input=samples/ata/bf.asm --output=-

test_valgrind: all
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./target/lc3 --input=samples/ata/bf.asm --output=-

all: parser lexer string_map lc3 cli
		mkdir -p target
		$(CC) $(CFLAGS) -o target/main.o -c src/main.c
		$(CC) $(CFLAGS) -o target/lc3 target/main.o target/lexer/lexer.o target/grammar/parser.o target/map/string_map.o target/cli/cli.o target/cli/default/default_cli.o target/_lc3/assembler/lc3assembler.o target/_lc3/assembler/lc3isa.o target/_lc3/assembler/lc3emulator.o target/_lc3/assembler/expecter.o -lfl

install: all
		cp target/lc3 /usr/local/bin/lc3

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

lc3: src/lc3/assembler/lc3assembler.c src/lc3/instructions/lc3isa.c src/lc3/emulator/lc3emulator.c
		 mkdir -p target/_lc3/assembler
		 $(CC) $(CFLAGS) -c src/lc3/assembler/lc3assembler.c -o target/_lc3/assembler/lc3assembler.o
		 $(CC) $(CFLAGS) -c src/lc3/instructions/lc3isa.c -o target/_lc3/assembler/lc3isa.o
		 $(CC) $(CFLAGS) -c src/lc3/emulator/lc3emulator.c -o target/_lc3/assembler/lc3emulator.o
		 $(CC) $(CFLAGS) -c src/lc3/expecter/expecter.c -o target/_lc3/assembler/expecter.o

cli: src/cli/cli.c src/cli/default/default_cli.c
		 mkdir -p target/cli
		 mkdir -p target/cli/default
		 $(CC) $(CFLAGS) -c src/cli/cli.c -o target/cli/cli.o
		 $(CC) $(CFLAGS) -c src/cli/default/default_cli.c -o target/cli/default/default_cli.o

clean:
	rm -rf target