#include <types.h>
#include <lib.h>
#include <hashtable.h>

void ht_initialize(struct htable *ht) {
    int i;
    for(i = 1; i < 10000; ++i)
        ht->id[i] = 0;
}

int ht_findempty(struct htable *ht) {
    int i;
    for (i = 1; i < 10000; i++) {
        if(!ht->id[i]) return i;
    }
    return -1;
}

int ht_setempty(struct htable *ht) {
    unsigned i;
    for(i = 1; i < 10000; i++) {
        if(ht->id[i] == 0) {
            ht->id[i] = 1;
            return i;
        }
    }
    return -1;
}

int ht_remove(struct htable *ht, int id) {
    if(ht->id[id] == 0) return -1;
    ht->id[id] = 0;
    return 0;
}

int ht_get_val(struct htable *ht, int id) {
    if(ht->id[id] == 1) return 1;
    return 0;
}
