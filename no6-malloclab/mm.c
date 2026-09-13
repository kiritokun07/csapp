/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * mm-naive.c - 最快、最浪费内存的 malloc 实现。
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 * 朴素做法：分配一块内存就是把 brk 指针往前推。块里只有有效载荷，
 * 没有 header/footer。块从不合并，也不复用。realloc 直接用
 * mm_malloc 和 mm_free 实现。
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * 给学生：请把这段头注释换成你自己方案的高层描述。
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)  /* 一次向 OS 扩展 4KB，减少 sbrk 次数 */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* 把 size 和 alloc 位打包进一个字 */
#define PACK(size, alloc) ((size) | (alloc))

/* p 指向一个字：按 4 字节读写。unsigned 避免位移时当成负数 */
#define GET(p)      (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* bp 是 payload 指针，也就是 malloc 返回给用户的那个地址 */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 * 给学生：动手之前，请先在下面的结构体里填写队伍信息。
 ********************************************************/
team_t team = {
    /* Team name */ /* 队名 */
    "kteam",
    /* First member's full name */ /* 第一位队员的全名 */
    "Kirito",
    /* First member's email address */ /* 第一位队员的邮箱 */
    "kiritokun@qq.com",
    /* Second member's full name (leave blank if none) */ /* 第二位队员的全名（没有则留空） */
    "",
    /* Second member's email address (leave blank if none) */ /* 第二位队员的邮箱（没有则留空） */
    ""
};

/* single word (4) or double word (8) alignment */ /* 单字（4）或双字（8）对齐 */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */ /* 向上取整到 ALIGNMENT 的最近倍数 */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static char *heap_listp;
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

static void *find_fit(size_t asize)
{
    void *bp = heap_listp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize)
            return bp;
    }
    return NULL;   /* 找不到，让 malloc 去 extend_heap */
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    if (csize - asize >= (2 * DSIZE)) {
        /* 余量够再放一个最小块：切开 */
        PUT(HDRP(bp), PACK(asize, 1));   /* ② 前半：已分配，这时才改 header */
        PUT(FTRP(bp), PACK(asize, 1));   /* ③ FTRP 用的是刚写下的 asize */
        bp = NEXT_BLKP(bp);              /* ④ 下一步跳 asize，落到余量起点 */
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {          /* 11：无需合并 */
        return bp;
    }
    else if (prev_alloc && !next_alloc) {    /* 10：只和后合并 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) {    /* 01：只和前合并 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        /* 先写下标：当前块的旧脚，再改前一块的头，bp 挪到前一块 */
        PUT(FTRP(bp), PACK(size, 0));             /* 当前块旧脚 = 新块的脚 */
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); /* 前一块的头 = 新块的头 */
        bp = PREV_BLKP(bp);
    }
    else {                                   /* 00：前后都并 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))
              + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        /* 新头 = 前一块的头；新脚 = 后一块的脚；bp 挪到前一块 */
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); /* 新头 */
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0)); /* 新脚 */
        bp = PREV_BLKP(bp);
    }
    return bp;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2 == 1) ? (words + 1) * WSIZE : words * WSIZE; /* 奇数则 words+1，再 * WSIZE */;
    bp = mem_sbrk(size);
    if (bp == (void *)-1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void *p = mem_sbrk( 4 * WSIZE);
    if (p == (void *)-1)
        return -1;
    heap_listp = (char *)p;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2 * WSIZE);
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 * mm_malloc - 通过把 brk 指针往前推来分配一块内存。
 *     分配出的块大小始终是对齐粒度的倍数。
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    if (size == 0)
        return NULL;
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
    bp = find_fit(asize);
    if (bp != NULL) {
        place(bp, asize);
        return bp;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    bp = extend_heap(extendsize / WSIZE);
    if (bp == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 * mm_free - 释放一块内存，当前实现什么都不做。
 */
void mm_free(void *bp)
{
    if (bp == NULL)
        return;
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 * mm_realloc - 简单地用 mm_malloc 和 mm_free 实现
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
