/* 
 * mystop.c - Another handy routine for testing your tiny shell
 * mystop.c - 用于测试 tiny shell 的辅助程序
 * 
 * usage: mystop <n>
 * 用法：mystop <n>
 * Sleeps for <n> seconds and sends SIGTSTP to itself.
 * 睡眠 <n> 秒后向自身发送 SIGTSTP。
 *
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
    pid_t pid; 

    if (argc != 2) {
	fprintf(stderr, "Usage: %s <n>\n", argv[0]);
	exit(0);
    }
    secs = atoi(argv[1]);

    for (i=0; i < secs; i++)
       sleep(1);
	
    pid = getpid(); 

    if (kill(-pid, SIGTSTP) < 0)
       fprintf(stderr, "kill (tstp) error");

    exit(0);

}
