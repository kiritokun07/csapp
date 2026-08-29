/* 
 * myspin.c - A handy program for testing your tiny shell 
 * myspin.c - 用于测试 tiny shell 的辅助程序
 * 
 * usage: myspin <n>
 * 用法：myspin <n>
 * Sleeps for <n> seconds in 1-second chunks.
 * 以每次 1 秒的方式睡眠 <n> 秒。
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) 
{
    int i, secs;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s <n>\n", argv[0]);
	exit(0);
    }
    secs = atoi(argv[1]);
    for (i=0; i < secs; i++)
	sleep(1);
    exit(0);
}
