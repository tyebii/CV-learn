#ifndef UNION_FIND_H
#define UNION_FIND_H

typedef struct {
    int *parent;
    int size;
    int capacity;
    int *rank;
} UnionFind;

/* creation / destruction */
UnionFind *uf_create(int capacity);
void uf_destroy(UnionFind *uf);

/* operations */
int uf_find(UnionFind *uf, int i);
void uf_unite(UnionFind *uf, int i, int j);
int uf_make_label(UnionFind *uf);

#endif