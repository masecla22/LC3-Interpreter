#ifndef STRING_MAP_H
#define STRING_MAP_H

#include <stdlib.h>
#include <stdio.h>

typedef struct TrieNode TrieNode;

typedef struct StringMap {
    TrieNode* root;
} StringMap;

struct TrieNode {
    TrieNode** children;
    void* value;
};

StringMap* stringMapCreate();
void stringMapPut(StringMap* map, const char* key, void* value);
void* stringMapGet(StringMap* map, char* key);
void stringMapRemove(StringMap* map, char* key);
void stringMapDestroy(StringMap* map, int freeValues);

void applyToAllValues(StringMap* map, void (*func)(void*));

#endif // STRING_MAP_H