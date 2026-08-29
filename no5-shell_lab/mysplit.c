/* 
 * mysplit.c - Another handy routine for testing your tiny shell
 * mysplit.c - 用于测试 tiny shell 的辅助程序
 * 
 * usage: mysplit <n>
 * 用法：mysplit <n>
 * Fork a child that spins for <n> seconds in 1-second chunks.
 * fork 出一个子进程，以每次 1 秒的方式空转 <n> 秒。
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char **argv) 
{
    int i, secs;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s <n>\n", argv[0]);
	exit(0);
    }
    secs = atoi(argv[1]);


    if (fork() == 0) { /* child */
                       /* 子进程 */
	for (i=0; i < secs; i++)
	    sleep(1);
	exit(0);
    }

    /* parent waits for child to terminate */
    /* 父进程等待子进程终止 */
    wait(NULL);

    exit(0);
}
