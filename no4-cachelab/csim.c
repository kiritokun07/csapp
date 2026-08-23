#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

typedef struct {
    int valid;
    unsigned long tag;
    int lru;
} line_t;
line_t *cache;          /* S*E 条 line，下标 = set*E + way */
int S, E_ways, s_bits, b_bits;
int hits, misses, evictions, stamp;

/* 不能叫 access：unistd.h 里已有同名函数 */
void cache_access(unsigned long addr)
{
    unsigned long set = (addr >> b_bits) & (S - 1);
    unsigned long tag = addr >> (s_bits + b_bits);
    line_t *lines = &cache[set * E_ways];
    int i;

    /* 1. 这个 set 里有没有身份证相同的 */
    for (i = 0; i < E_ways; i++) {
        if (lines[i].valid && lines[i].tag == tag) {
            hits++;
            lines[i].lru = ++stamp;
            return;
        }
    }

    /* 2. 没有 → miss */
    misses++;

    /* 3. 有空位就占用 */
    for (i = 0; i < E_ways; i++) {
        if (!lines[i].valid) {
            lines[i].valid = 1;
            lines[i].tag = tag;
            lines[i].lru = ++stamp;
            return;
        }
    }

    /* 4. 没空位：赶走 lru 最小的（最久没用） */
    evictions++;
    int vic = 0;
    for (i = 1; i < E_ways; i++) {
        if (lines[i].lru < lines[vic].lru)
            vic = i;
    }
    lines[vic].tag = tag;
    lines[vic].lru = ++stamp;
}

int main(int argc, char *argv[])
{
    int s = 0, E = 0, b = 0, opt;
    char *tracefile = NULL;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
        case 's': s = atoi(optarg); break;
        case 'E': E = atoi(optarg); break;
        case 'b': b = atoi(optarg); break;
        case 't': tracefile = optarg; break;
        case 'v': /* 先忽略 verbose */ break;
        default:
            return 1;
        }
    }
    s_bits = s;
    S = 1 << s;
    E_ways = E;
    b_bits = b;
    cache = calloc(S * E_ways, sizeof(line_t));  /* calloc 自带 valid=0 */
    if (!cache) {
        fprintf(stderr, "calloc failed\n");
        return 1;
    }
    FILE *fp = fopen(tracefile, "r");
    char op;
    unsigned long addr;
    int size;
    while (fscanf(fp, " %c %lx,%d", &op, &addr, &size) == 3) {
        if (op == 'I')
            continue;
        if (op == 'M') {
            cache_access(addr);
            cache_access(addr);
        } else {          /* L 或 S */
            cache_access(addr);
        }
    }
    fclose(fp);
    printSummary(hits, misses, evictions);
    free(cache);
    return 0;
}
