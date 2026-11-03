#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Entry {
    int key;
    int value;
    int next;
} Entry;

typedef struct Map {
    Entry *entries;
    int *first;
    int bucket_count;
    int entries_size;
    int entries_count; 
    int first_free_entry;      
} Map;

void rehash(Map *m);

void put(int key, int value, Map *m);

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
    Map nm = {.entries=calloc(m->entries_size, sizeof(Entry)), .first=calloc(2 * m->bucket_count,
         sizeof(int)), .bucket_count=2 * m->bucket_count, .entries_size=m->entries_size,
          .entries_count=0, .first_free_entry=0};
    
    if (nm.entries == NULL || nm.first == NULL) {
        free(nm.entries);
        free(nm.first);
        printf("erreur");
        return;
    }

    for(int i = 0; i < nm.bucket_count; i++) {
        nm.first[i] = -1;
    }

    for(int i = 0; i < m->bucket_count; i++) {
        int cur = m->first[i];
        while(cur != -1) {
            Entry *e = &(m->entries[cur]);
            int index = nm.first[slot(e->key, &nm)];
            int ffe = nm.first_free_entry;

            nm.entries[ffe] = (Entry){e->key, e->value, index};
            nm.first[slot(e->key, &nm)] = ffe;
            cur = e->next;

            nm.first_free_entry++;
            nm.entries_count++;
        }
    }
    free(m->entries);
    free(m->first);
    *m = nm;
}

float load_factor(Map *m) {
    return (float)(m->entries_count) / m->bucket_count;
}

void put(int key, int value, Map *m) {

    Entry e = {.key=key, .value=value, .next=-1};
    int current = m->first[slot(key, m)];
    
    while (current != -1) {
        if (m->entries[current].key == key) {
            m->entries[current].value = value;
            return;
        } 
        current = m->entries[current].next;
    }

    if ((load_factor(m) + 1.0 / m->bucket_count) > 0.75) {
        rehash(m);
    }

    int index = m->first[slot(key, m)];
    int x = m->first_free_entry;

    m->entries[x] = e;
    m->entries[x].next = index;
    m->first[slot(key, m)] = x;
    m->first_free_entry++;
    m->entries_count++;
}

int *get(int key, Map *m) {
    if (m->first[slot(key, m)] == -1) {
        return NULL;
    }
    Entry *current = &(m->entries[m->first[slot(key, m)]]);
    while (current->next != -1) {
        if (current->key == key) {
            return &(current->value);
        }
        current = &(m->entries[current->next]);
    }
    if (current->key == key) {
        return &(current->value);
    }
    return NULL;
}

int main(void) {
    Map m = {.entries = malloc((2 << 8) * sizeof(Entry)), .first = malloc(2 * sizeof(int)),
         .bucket_count=2, .entries_count=0, .entries_size= 2 << 8, .first_free_entry=0};
    for(int i=0;i<m.bucket_count;i++) {
        m.first[i] = -1;
    }  
    for(int i = 0; i < 100; i++) {
        put(i, i, &m);
        printf("%d\n", m.bucket_count);
    }
    return 0;
}