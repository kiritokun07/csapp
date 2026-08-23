/* 
 * trans.c - Matrix transpose B = A^T
 * trans.c - 矩阵转置 B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 * 每个转置函数的原型必须形如：
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 * 评测方式：在 1KB 直接映射缓存、块大小为 32 字节的条件下，
 * 统计转置函数产生的 cache miss 次数。
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 * transpose_submit - 这是作业 Part B 用于评分的转置函数。
 *     不要修改描述字符串 "Transpose submission"，因为驱动会
 *     搜索该字符串来识别需要评分的转置函数。
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, t0, t1, t2, t3, t4, t5, t6, t7;
    if (M == 32 && N == 32) {
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (ii = i; ii < i + 8; ii++) {
                    t0 = A[ii][j + 0];
                    t1 = A[ii][j + 1];
                    t2 = A[ii][j + 2];
                    t3 = A[ii][j + 3];
                    t4 = A[ii][j + 4];
                    t5 = A[ii][j + 5];
                    t6 = A[ii][j + 6];
                    t7 = A[ii][j + 7];
                    B[j + 0][ii] = t0;
                    B[j + 1][ii] = t1;
                    B[j + 2][ii] = t2;
                    B[j + 3][ii] = t3;
                    B[j + 4][ii] = t4;
                    B[j + 5][ii] = t5;
                    B[j + 6][ii] = t6;
                    B[j + 7][ii] = t7;
                }
            }
        }
    } else if (M == 64 && N == 64) {
        /* 这里再写 64 的 4×4*/
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                /* 上 4 行：P 就位，Q 寄存在 R 的位置 */
                for (ii = 0; ii < 4; ii++) {
                    t0 = A[i+ii][j+0]; t1 = A[i+ii][j+1];
                    t2 = A[i+ii][j+2]; t3 = A[i+ii][j+3];
                    t4 = A[i+ii][j+4]; t5 = A[i+ii][j+5];
                    t6 = A[i+ii][j+6]; t7 = A[i+ii][j+7];
                    B[j+0][i+ii] = t0; B[j+1][i+ii] = t1;
                    B[j+2][i+ii] = t2; B[j+3][i+ii] = t3;
                    B[j+0][i+ii+4] = t4; B[j+1][i+ii+4] = t5;
                    B[j+2][i+ii+4] = t6; B[j+3][i+ii+4] = t7;
                }
                /* 下 4 行：取出 Q，写入 R、Q、S */
                for (ii = 0; ii < 4; ii++) {
                    /* 这条 B 行上寄放的 Q，4 个 int 同一条 line */
    t0 = B[j+ii][i+4];
    t1 = B[j+ii][i+5];
    t2 = B[j+ii][i+6];
    t3 = B[j+ii][i+7];
    /* R 的一列 */
    t4 = A[i+4][j+ii];
    t5 = A[i+5][j+ii];
    t6 = A[i+6][j+ii];
    t7 = A[i+7][j+ii];
    /* R 就位（还是这一条 B 行） */
    B[j+ii][i+4] = t4;
    B[j+ii][i+5] = t5;
    B[j+ii][i+6] = t6;
    B[j+ii][i+7] = t7;
    /* Q 就位；这时才写 B[j+4]，B[j] 可以被踢 */
    B[j+4+ii][i+0] = t0;
    B[j+4+ii][i+1] = t1;
    B[j+4+ii][i+2] = t2;
    B[j+4+ii][i+3] = t3;
    /* S */
    t0 = A[i+4][j+4+ii];
    t1 = A[i+5][j+4+ii];
    t2 = A[i+6][j+4+ii];
    t3 = A[i+7][j+4+ii];
    B[j+4+ii][i+4] = t0;
    B[j+4+ii][i+5] = t1;
    B[j+4+ii][i+6] = t2;
    B[j+4+ii][i+7] = t3;
                }
            }
        }
    } else {
        /* 61×67 */
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (ii = i; ii < i + 8 && ii < N; ii++) {
                    for (t0 = j; t0 < j + 8 && t0 < M; t0++) {
                        t1 = A[ii][t0];
                        B[t0][ii] = t1;
                    }
                }
            }
        }
    }
    
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 * 你可以在下方定义额外的转置函数。我们已经定义了一个
 * 简单版本，方便你上手。
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 * trans - 简单的基准转置函数，未针对 cache 做优化。
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 * registerFunctions - 向驱动注册你的转置函数。
 *     运行时，驱动会评测每个已注册函数并汇总其性能。
 *     这是试验不同转置策略的便捷方式。
 */
void registerFunctions()
{
    /* Register your solution function */
    /* 注册你的解答函数 */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    /* 注册任何额外的转置函数 */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 * is_transpose - 辅助函数，检查 B 是否为 A 的转置。
 *     你可以在转置函数返回前调用它，以检查转置是否正确。
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
