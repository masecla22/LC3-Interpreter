#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli/default/default_cli.h"
#include "lc3/assembler/lc3assembler.h"
#include "lc3/context/lc3context.h"
#include "lc3/emulator/lc3emulator.h"

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

void handleRandomization(int* seedOutput, int* randomizedOutput) {
    int randomized = stringMapGet(result.flags, "randomized") != NULL;
    int seed = 0;
    char* seedStr = (char*)stringMapGet(result.flags, "seed");
    if (seedStr != NULL) {
        seed = atoi(seedStr);
    } else {
        seed = 0;
    }

    if (randomized) {
        // Handle it here, no need to pass it to the LC3
        srand(seed);
        *seedOutput = seed;
        *randomizedOutput = randomized;
    } else {
        *seedOutput = 0;
        *randomizedOutput = 0;
    }
}

FILE* getInputFile() {
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

    return input;
}

FILE* getOutputFile() {
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

    return output;
}

int main(int argc, char** argv) {
    atexit(destroyParser);

    parser = defaultCLIParserCreate(argc, argv);
    result = cliParserParse(parser);

    if (stringMapGet(result.flags, "help") != NULL) {
        printHelpMessage(parser, stdout);
        exit(0);
    }

    FILE* input = getInputFile();
    FILE* output = getOutputFile();

    int seed = 0;
    int randomized = 0;

    handleRandomization(&seed, &randomized);

    // Check if we are assembling or emulating
    int onlyAssemble = stringMapGet(result.flags, "assemble") != NULL;
    int onlyEmulate = stringMapGet(result.flags, "emulate") != NULL;

    // Free the memory allocated by the CLI parser
    destroyParser();

    if (onlyAssemble && onlyEmulate) {
        fprintf(stderr, "To both emulate and assemble omit both flags.\n");
        exit(1);
    }

    if (onlyAssemble) {
        LC3Context context = {input, output, randomized, seed};
        LC3EmulatorState emulatorState = assemble(context);

        // Dump the memory to the output file.
        dumpToFile(&emulatorState, output);
    } else if (onlyEmulate) {
        LC3EmulatorState emulatorState = loadFromFile(input);
        emulate((LC3Context){input, output, randomized, seed}, emulatorState);
    } else {
        LC3Context context = {input, output, randomized, seed};
        LC3EmulatorState emulatorState = assemble(context);

        // Run the emulator
        emulate(context, emulatorState);

        // Free the memory
        free(emulatorState.memory);
        emulatorState.memory = NULL;
    }

    // Close the files
    if (input != stdin) {
        fclose(input);
    }
    if (output != stdout) {
        fclose(output);
    }
}
