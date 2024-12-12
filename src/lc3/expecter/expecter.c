#include "expecter.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void stringToLower(char* string) {
    for (int i = 0; string[i] != '\0'; i++) {
        string[i] = tolower(string[i]);
    }
}

EmulatorExpectations loadExpectationFromFile(FILE* expectationsFile) {
    EmulatorExpectations expectations = {0};

    char buffer[256];

    // Read each line
    while (fgets(buffer, 256, expectationsFile) != NULL) {
        // If the line is empty skip it
        if (strlen(buffer) < 2) {
            continue;
        }

        // Remove the newline character
        if (buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = '\0';

        // To lower
        stringToLower(buffer);

        char* firstSpace = strchr(buffer, ' ');
        if (firstSpace == NULL) {
            fprintf(stderr, "Invalid line in expectations file: %s\n", buffer);
            exit(1);
        }

        firstSpace[0] = '\0';

        char* action = buffer;
        if (strcmp(action, "put") == 0) {
            // This is a put action
            char* location = firstSpace + 1;

            // Get the value
            char* value = strchr(location, ' ');
            if (value == NULL) {
                fprintf(stderr, "Invalid put action in expectations file: %s\n", buffer);
                exit(1);
            }

            value[0] = '\0';

            int valueNumber = atoi(value + 1);

            // Figure out if it's a register or memory location
            if (location[0] == 'R' || location[0] == 'r') {
                // This is a register
                int registerNumber = atoi(location + 1);
                expectations.input.replaceRegisters[registerNumber] = 1;
                expectations.input.registerReplacements[registerNumber] = valueNumber;
            } else if (location[0] == 'x' || location[0] == 'X') {
                // This is a memory location
                int memoryLocation = strtol(location + 1, NULL, 16);
                expectations.input.replaceMemory[memoryLocation] = 1;
                expectations.input.memoryReplacements[memoryLocation] = valueNumber;
            } else {
                fprintf(stderr, "PUT: Invalid location in expectations file: %s\n", location);
            }
        } else if (strcmp(action, "expect") == 0) {
            // This is an expect action
            char* location = firstSpace + 1;

            // Figure out if it's a register or memory location
            if (location[0] == 'R' || location[0] == 'r') {
                // This is a register
                int registerNumber = atoi(location + 1);
                expectations.output.expectedRegisters[registerNumber] = 1;
            } else if (location[0] == 'x' || location[0] == 'X') {
                // This is a memory location
                int memoryLocation = strtol(location + 1, NULL, 16);
                expectations.output.expectedMemory[memoryLocation] = 1;
            } else {
                fprintf(stderr, "EXPECT: Invalid location in expectations file: %s\n", location);
            }

        } else {
            fprintf(stderr, "Invalid action in expectations file: %s\n", action);
        }
    }

    return expectations;
}