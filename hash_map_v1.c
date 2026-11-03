#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Entry {
    int key;
    int value;
} Entry;

typedef struct Bucket {
    Entry *entries;
    int length;
} Bucket;

typedef struct Map {
    Bucket *buckets;
    int bucket_count;
    int entries_count;        
} Map;

uint64_t hash(uint64_t x) { // https://github.com/svaarala/duktape/blob/master/misc/splitmix64.c
	uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	return z ^ (z >> 31);
}

int slot(int key, Map *m) {
    return hash(key) % m->bucket_count;
}

void rehash(Map *m) {
    int ns = 2 * m->bucket_count;
    Map nm = {.buckets=calloc(ns, sizeof(Bucket)), .bucket_count=(m->bucket_count * 2), .entries_count=m->entries_count};
    if (nm.buckets == NULL) {
        printf("erreur");
        return;
    }
    for(int i = 0; i < m->bucket_count; i++) {
        for(int j = 0; j < m->buckets[i].length; j++) {
            Entry e = m->buckets[i].entries[j];
            int dest = slot(e.key, &nm);
            Bucket *b = &nm.buckets[dest];
            Entry *tmp = realloc(b->entries, (b->length + 1)* sizeof(Entry)); //tester avec un pointeur temporaire si realloc vaut NULL
            if (tmp == NULL) {
                printf("erreur");
                for(int k = 0; k < nm.bucket_count; k++) {
                    free(nm.buckets[k].entries);
                }
                free(nm.buckets);
                return;
            }
            b->entries = tmp;
            b->entries[b->length] = e;
            b->length++;
        } 
    }
    for(int i = 0; i < m->bucket_count; i++) {
        free(m->buckets[i].entries);
    }
    free(m->buckets);
    *m = nm;
}

float load_factor(Map *m) {
    return (float)(m->entries_count) / m->bucket_count;
}

void put(int key, int value, Map *m) { 
    Entry e = {.key=key, .value=value};
    Bucket *b = &(m->buckets[slot(key, m)]);
    for (int i = 0; i < b->length; i++) {
        if (b->entries[i].key == key) {
            b->entries[i].value = value;
            return;
        }
    }
    if ((load_factor(m) + 1.0 / m->bucket_count) > 0.75) {
        rehash(m);
        b = &(m->buckets[slot(key, m)]); 
    }
    Entry *tmp = realloc(b->entries, (b->length + 1) * sizeof(Entry));
    if (tmp == NULL) { 
        printf("error");
        return;
    }
    b->entries = tmp;
    b->entries[b->length] = e;
    b->length++; 
    m->entries_count++;
    }

int *get(int key, Map *m) {
    Bucket b = m->buckets[slot(key, m)];
    for(int i = 0; i < b.length; i++) {
        if (b.entries[i].key == key)
            return &(b.entries[i].value);
    }
    return NULL;
}

int main(void) {
    return 0;
}