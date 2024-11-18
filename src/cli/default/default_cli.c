#include "../cli.h"

CLIParser* defaultCLIParserCreate(int argc, char** argv) {
    CLIParser* parser = cliParserCreate(argc, argv, "\nLC3 Toolchain by Matt\nVersion 1.0.0");

    cliParserAddNoValueFlag(parser, "help", "Prints the help message", 'h');
    cliParserAddNoValueFlag(parser, "randomized", "Randomizes the LC3 memory space before execution", 'r');
    cliParserAddValueFlag(parser, "seed", "Sets the seed for the random number generator", 's', "seed");

    cliParserAddValueFlag(parser, "input", "Sets the input file (- for stdin)", 'i', "file");
    cliParserAddValueFlag(parser, "output", "Sets the output file (- for stdout)", 'o', "file");

    return parser;
}
