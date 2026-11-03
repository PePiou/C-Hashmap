#include <stdio.h>
#include <stdbool.h>

typedef struct Entry {
    char *key;
    int value;
    bool used;
} Entry;

typedef struct Map {
    Entry buckets[8];        
} Map;

int hash(char *s, int len) {
    int result = 0;
    for(int i = 0; i < len; i++) {
        result += s[i];
    }
    return result;
}

int slot(char *key) {
    return hash(key, 3) % 8;
}

void put(char *key, int value, Map *m) { 
    Entry e = {.key=key, .value=value, .used=true};
    m->buckets[slot(key)] = e;
}

int *get(char *key, Map *m) {
    int bucket = slot(key); 
    Entry e = m->buckets[bucket];
    return m->buckets[bucket].used && m->buckets[bucket].key == e.key ? &(m->buckets[bucket].value) : NULL;
}

int main(void) {

    char *s = "abc";
    int len = 3;

    Entry e = {.key=s, .value=9, .used=true};
    Map m = {0};

    return 0;
}