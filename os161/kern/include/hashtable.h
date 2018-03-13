#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

struct htable {
    unsigned char id[10000];
};

int ht_initialize(struct htable *ht);

int ht_findempty(struct htable *ht);

int ht_setempty(struct htable *ht);

int ht_remove(struct htable *ht, int id);

#endif /* HASHTABLE_H */
