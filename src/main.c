#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli/default/default_cli.h"
#include "lc3/context/lc3context.h"

CLIParser* parser = NULL;
CLIParseResult result = {NULL};

void destroyParser(void) {
    if (parser != NULL) {
        cliParserDestroy(parser);
        cliParserResultDestroy(result);

        parser = NULL;
        result.flags = NULL;
    }
}

int main(int argc, char** argv) {
    atexit(destroyParser);

    parser = defaultCLIParserCreate(argc, argv);
    result = cliParserParse(parser);

    if (stringMapGet(result.flags, "help") != NULL) {
        printHelpMessage(parser, stdout);
        exit(0);
    }

    char* inputFile = (char*)stringMapGet(result.flags, "input");

    FILE* input = NULL;
    if (inputFile == NULL || strcmp(inputFile, "-") == 0) {
        input = stdin;
    } else {
        input = fopen(inputFile, "r");
        if (input == NULL) {
            fprintf(stderr, "Could not open input file: %s\n", inputFile);
            exit(1);
        }
    }

    char* outputFile = (char*)stringMapGet(result.flags, "output");
    FILE* output = NULL;
    if (outputFile == NULL || strcmp(outputFile, "-") == 0) {
        output = stdout;
    } else {
        output = fopen(outputFile, "w");
        if (output == NULL) {
            fprintf(stderr, "Could not open output file: %s\n", outputFile);
            exit(1);
        }
    }

    int randomized = stringMapGet(result.flags, "randomized") != NULL;
    int seed = 0;
    char* seedStr = (char*)stringMapGet(result.flags, "seed");
    if (seedStr != NULL) {
        seed = atoi(seedStr);
    } else {
        seed = 0;
    }

    // Free the memory allocated by the parser
    destroyParser();

    if (randomized) {
        // Handle it here, no need to pass it to the LC3
        srand(seed);
    }

    LC3Context context = {input, output, randomized, seed};
    
}