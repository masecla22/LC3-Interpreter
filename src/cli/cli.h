#ifndef CLI_H
#define CLI_H

#include "../map/string_map.h"

typedef struct CLIFlagDefinition CLIFlagDefinition;

typedef struct CLIParser {
    int argc;
    char** argv;

    StringMap* declaredFlags;

    CLIFlagDefinition* flags[256]; // Mapping from short name to flag definition

    const char* footer;
} CLIParser;

typedef struct CLIFlagDefinition {
    const char* name;
    const char* description;
    char shortName;
    const char* valueName;

    int hasValue;
} CLIFlagDefinition;

typedef struct CLIParseResult {
    StringMap* flags;
} CLIParseResult;

CLIParser* cliParserCreate(int argc, char** argv, const char* footer);
void cliParserDestroy(CLIParser* parser);
void cliParserAddFlag(CLIParser* parser, const char* name, const char* description, const char shortName, const char* valueName, int hasValue);

void cliParserAddNoValueFlag(CLIParser* parser, const char* name, const char* description, const char shortName);
void cliParserAddValueFlag(CLIParser* parser, const char* name, const char* description, const char shortName, const char* valueName);

CLIParseResult cliParserParse(CLIParser* parser);
void cliParserResultDestroy(CLIParseResult result);

void printHelpMessage(CLIParser* parser, FILE* stream);

#endif // CLI_H