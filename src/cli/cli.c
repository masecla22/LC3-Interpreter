#include "cli.h"

#include <string.h>

#include "../map/string_map.h"

const char NON_NULL_STRING[2] = "";

CLIParser* cliParserCreate(int argc, char** argv, const char* footer) {
    CLIParser* parser = (CLIParser*)calloc(1, sizeof(CLIParser));
    parser->argc = argc;
    parser->argv = argv;
    parser->declaredFlags = stringMapCreate();
    parser->footer = footer;
    return parser;
}

void cliParserDestroy(CLIParser* parser) {
    stringMapDestroy(parser->declaredFlags, 1);
    free(parser);
}

void cliParserAddFlag(CLIParser* parser, const char* name, const char* description, const char shortName, const char* valueName, int hasValue) {
    if (parser->flags[(unsigned char)shortName] != NULL) {
        fprintf(stderr, "Flag with short name %c already exists\n", shortName);
        exit(1);
    }

    CLIFlagDefinition* flag = (CLIFlagDefinition*)malloc(sizeof(CLIFlagDefinition));
    flag->name = name;
    flag->description = description;
    flag->shortName = shortName;
    flag->valueName = valueName;
    flag->hasValue = hasValue;
    stringMapPut(parser->declaredFlags, name, flag);
    parser->flags[(unsigned char)shortName] = flag;
}

void cliParserAddNoValueFlag(CLIParser* parser, const char* name, const char* description, const char shortName) {
    cliParserAddFlag(parser, name, description, shortName, NULL, 0);
}

void cliParserAddValueFlag(CLIParser* parser, const char* name, const char* description, const char shortName, const char* valueName) {
    cliParserAddFlag(parser, name, description, shortName, valueName, 1);
}

CLIParseResult cliParserParse(CLIParser* parser) {
    CLIParseResult result = {stringMapCreate()};

    for (int i = 1; i < parser->argc; i++) {
        char* arg = parser->argv[i];

        if (strncmp(arg, "--", 2) == 0) {
            arg += 2;
            char* value = strchr(arg, '=');
            if (value != NULL) {
                *value = '\0';
                value++;
            }

            CLIFlagDefinition* flag = (CLIFlagDefinition*)stringMapGet(parser->declaredFlags, arg);
            if (flag == NULL) {
                fprintf(stderr, "Unknown flag: %s\n", arg);
                fprintf(stderr, "Use --help to see available flags\n");
                
                cliParserResultDestroy(result);
                exit(1);
            }

            if (flag->hasValue) {
                if (value == NULL) {
                    fprintf(stderr, "Flag %s requires a value specified with '='\n", arg);
                    cliParserResultDestroy(result);
                    exit(1);
                }
                stringMapPut(result.flags, arg, value);
            } else {
                if (value != NULL) {
                    fprintf(stderr, "Flag %s does not take a value\n", arg);
                    cliParserResultDestroy(result);
                    exit(1);
                }
                stringMapPut(result.flags, arg, (char*) NON_NULL_STRING);
            }
        } else if (arg[0] == '-') {
            arg++;  // Wipe the - at the beginning

            int allowsValue = strlen(arg) == 1;  // Only allow values for single character flags

            while (*arg != '\0') {
                CLIFlagDefinition* flag = parser->flags[(unsigned char)*arg];
                if (flag == NULL) {
                    fprintf(stderr, "Unknown flag: %c\n", *arg);
                    cliParserResultDestroy(result);
                    exit(1);
                }

                if (flag->hasValue && !allowsValue) {
                    fprintf(stderr, "Flag '%c' cannot be used in short-hand form with other flags\n", *arg);
                    cliParserResultDestroy(result);
                    exit(1);
                }

                if (flag->hasValue) {
                    if (arg[1] != '\0') {
                        fprintf(stderr, "Flag '%c' requires a value\n", *arg);
                        cliParserResultDestroy(result);
                        exit(1);
                    }

                    if (i + 1 >= parser->argc) {
                        fprintf(stderr, "Flag '%c' requires a value\n", *arg);
                        cliParserResultDestroy(result);
                        exit(1);
                    }

                    if (parser->argv[i + 1][0] == '-') {
                        fprintf(stderr, "Flag '%c' requires a value\n", *arg);
                        cliParserResultDestroy(result);
                        exit(1);
                    }

                    stringMapPut(result.flags, flag->name, parser->argv[++i]);
                } else {
                    stringMapPut(result.flags, flag->name, (char*) NON_NULL_STRING);
                }

                arg++;
            }
        }
    }

    return result;
}

void printArgument(void* cliFlagDefinition) {
    CLIFlagDefinition* flag = (CLIFlagDefinition*)cliFlagDefinition;
    if (flag->hasValue) {
        printf("  -%c %s, --%s=%s: %s\n", flag->shortName, flag->valueName, flag->name, flag->valueName, flag->description);
    } else {
        printf("  -%c, --%s: %s\n", flag->shortName, flag->name, flag->description);
    }
}

void printHelpMessage(CLIParser* parser, FILE* stream) {
    fprintf(stream, "Usage: %s [flags]\n", parser->argv[0]);
    fprintf(stream, "Flags:\n");

    applyToAllValues(parser->declaredFlags, printArgument);

    if (parser->footer != NULL) {
        fprintf(stream, "%s\n", parser->footer);
    }
}

void cliParserResultDestroy(CLIParseResult result) {
    stringMapDestroy(result.flags, 0);
}