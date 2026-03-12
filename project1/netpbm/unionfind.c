#include "unionfind.h"
#include <stdio.h>
#include <stdlib.h>

UnionFind *uf_create(int capacity)
{
    UnionFind *uf = malloc(sizeof *uf);
    if (!uf) return NULL;

    uf->size = 1;
    uf->capacity = capacity+1;
    uf->parent = malloc(uf->capacity * sizeof *uf->parent);
    uf->rank = malloc(uf->capacity * sizeof *uf->rank);

    for (int i = 0; i < capacity; i++) {
        uf->parent[i] = i;
        uf->rank[i] = 0;
    }

    return uf;
}

int uf_make_label(UnionFind *uf)
{
    if (uf->size == uf->capacity) {
        uf->capacity *= 2;
        uf->parent =
            realloc(uf->parent,
                    uf->capacity * sizeof *uf->parent);
        uf->rank =
            realloc(uf->rank,
                    uf->capacity * sizeof *uf->rank);
    }

    int label = uf->size;
    uf->parent[label] = label;
    uf->rank[label] = 0;
    uf->size++;

    return label;
}

int uf_find(UnionFind *uf, int i)
{
    if (uf->parent[i] != i)
        uf->parent[i] = uf_find(uf, uf->parent[i]);

    return uf->parent[i];
}

void uf_unite(UnionFind *uf, int i, int j)
{
    int irep = uf_find(uf, i);
    int jrep = uf_find(uf, j);

    if (irep == jrep) return;

    if (uf->rank[irep] < uf->rank[jrep])
        uf->parent[irep] = jrep;
    else if (uf->rank[irep] > uf->rank[jrep])
        uf->parent[jrep] = irep;
    else {
        uf->parent[irep] = jrep;
        uf->rank[jrep]++;
    }
}

void uf_destroy(UnionFind *uf)
{
    if (!uf) return;
    free(uf->rank);
    free(uf->parent);
    free(uf);
}