#include "string_map.h"

TrieNode* trieNodeCreate() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    node->children = NULL;
    node->value = NULL;
    return node;
}

TrieNode* trieNodeGetChild(TrieNode* node, char c) {
    if(node->children == NULL) {
        node->children = (TrieNode**)calloc(256, sizeof(TrieNode*));
    }

    unsigned char uc = (unsigned char)c;
    if(node->children[uc] == NULL) {
        node->children[uc] = trieNodeCreate();
    }

    return node->children[uc];
}

StringMap* stringMapCreate() {
    StringMap* map = (StringMap*)malloc(sizeof(StringMap));
    map->root = trieNodeCreate();
    return map;
}

void stringMapPut(StringMap* map, const char* key, void* value) {
    TrieNode* node = map->root;
    for(int i = 0; key[i] != '\0'; i++) {
        node = trieNodeGetChild(node, key[i]);
    }
    node->value = value;
}

void* stringMapGet(StringMap* map, char* key) {
    TrieNode* node = map->root;
    for(int i = 0; key[i] != '\0'; i++) {
        if(node->children == NULL || node->children[(unsigned char)key[i]] == NULL) {
            return NULL;
        }
        node = node->children[(unsigned char)key[i]];
    }
    return node->value;
}

void stringMapRemove(StringMap* map, char* key) {
    TrieNode* node = map->root;
    for(int i = 0; key[i] != '\0'; i++) {
        if(node->children == NULL || node->children[(unsigned char)key[i]] == NULL) {
            return;
        }
        node = node->children[(unsigned char)key[i]];
    }
    node->value = NULL;
}

void trieNodeDestroy(TrieNode* node, int freeValues) {
    if(node->children != NULL) {
        for(int i = 0; i < 256; i++) {
            if(node->children[i] != NULL) {
                trieNodeDestroy(node->children[i], freeValues);
            }
        }
        free(node->children);
    }

    if(freeValues && node->value != NULL) {
        free(node->value);
    }

    free(node);
}

void stringMapDestroy(StringMap* map, int freeValues) {
    if(map == NULL) {
        return;
    }

    trieNodeDestroy(map->root, freeValues);
    free(map);
}



void applyToAllValuesRec(TrieNode* node, void (*func)(void*)) {
    if(node->value != NULL) {
        func(node->value);
    }

    if(node->children != NULL) {
        for(int i = 0; i < 256; i++) {
            if(node->children[i] != NULL) {
                applyToAllValuesRec(node->children[i], func);
            }
        }
    }
}

void applyToAllValues(StringMap* map, void (*func)(void*)) {
    applyToAllValuesRec(map->root, func);
}
