#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Entry {
    int key;
    int value;
    struct Entry *next;
} Entry;

typedef struct Map {
    Entry **entries;
    int bucket_count;
    int entries_count;
} Map;

void rehash(Map *m); 
void put(int key, int value, Map *m);

float loadfactor(Map *m) {
    return (float)m->entries_count / m->bucket_count;
}

uint64_t hash(uint64_t x) { // https://github.com/svaarala/duktape/blob/master/misc/splitmix64.c
    uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

int slot(int key, Map *m) {
    return hash(key) % m->bucket_count; 
}

int *get(int key, Map *m) {
    Entry *pnt = m->entries[slot(key, m)];
    while (pnt != NULL) {
        if (pnt->key == key) {
            return &(pnt->value);
        }
        pnt = pnt->next;
    }
    return NULL;
}

void put(int key, int value, Map *m) {
    Entry *e = m->entries[slot(key, m)];
    while (e != NULL) {
        if (e->key == key) {
            e->value = value;
            return;
        }
        e = e->next;
    }
    Entry *new = malloc(sizeof *new);
    if (new == NULL) {
        fprintf(stderr, "Erreur d'allocation\n");
        return;
    }
    if (loadfactor(m) + 1.0 / m->bucket_count > 0.75) {
        rehash(m);
    }
    *new = (Entry){
        .key = key,
        .value = value,
        .next = m->entries[slot(key, m)]
    };
    m->entries[slot(key, m)] = new;
    m->entries_count++;
}

void rehash(Map *m) {
    Map nm = {calloc(m->bucket_count * 2, sizeof(Entry *)), m->bucket_count * 2, m->entries_count};
    if (nm.entries == NULL) {
        printf("erreur");
        return;
    }
    for(int i = 0; i < m->bucket_count; i++) {
        Entry *pnt = m->entries[i]; 
        while (pnt != NULL) {
            Entry *next = pnt->next;
            pnt->next = nm.entries[slot(pnt->key, &nm)];
            nm.entries[slot(pnt->key, &nm)] = pnt;
            pnt = next;
        } 
    } 
    free(m->entries);
    *m = nm;
}

Map init(int size) {
    return (Map){.entries=calloc(size, sizeof(Entry*)), .bucket_count=size, .entries_count=0};
}

void destroy(Map *m) {
    for(int i = 0; i < m->bucket_count; i++) {
        Entry *pnt = m->entries[i];
        while(pnt != NULL) {
            Entry *next = pnt->next;
            free(pnt);
            pnt = next;
        }
    }
    free(m->entries);
    free(m);
}

int main(void) {
    Map m = init(2);

    for(int i=0; i<100; i++) {
        put(i, i, &m);
        printf("%d\n", m.bucket_count);
    }

    printf("%d", *get(76, &m));
    
    destroy(&m);
    
    return 0;
}