/* 
 * tsh - A tiny shell program with job control
 * tsh - 带作业控制的微型 shell 程序
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
/* 杂项常量 */
#define MAXLINE    1024   /* max line size */
                          /* 最大行长度 */
#define MAXARGS     128   /* max args on a command line */
                          /* 命令行最大参数个数 */
#define MAXJOBS      16   /* max jobs at any point in time */
                          /* 同一时刻的最大作业数 */
#define MAXJID    1<<16   /* max job ID */
                          /* 最大作业 ID */

/* Job states */
/* 作业状态 */
#define UNDEF 0 /* undefined */
                /* 未定义 */
#define FG 1    /* running in foreground */
                /* 前台运行 */
#define BG 2    /* running in background */
                /* 后台运行 */
#define ST 3    /* stopped */
                /* 已停止 */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 *
 * 作业状态：FG（前台）、BG（后台）、ST（停止）
 * 作业状态转换及触发动作：
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg 命令
 *     ST -> BG  : bg 命令
 *     BG -> FG  : fg 命令
 * 同一时刻最多只能有 1 个作业处于 FG 状态。
 */

/* Global variables */
/* 全局变量 */
extern char **environ;      /* defined in libc */
                            /* 在 libc 中定义 */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
                            /* 命令行提示符（不要修改） */
int verbose = 0;            /* if true, print additional output */
                            /* 为真时打印额外诊断信息 */
int nextjid = 1;            /* next job ID to allocate */
                            /* 下一个要分配的作业 ID */
char sbuf[MAXLINE];         /* for composing sprintf messages */
                            /* 用于拼接 sprintf 消息 */

struct job_t {              /* The job struct */
                            /* 作业结构体 */
    pid_t pid;              /* job PID */
                            /* 作业的进程 ID */
    int jid;                /* job ID [1, 2, ...] */
                            /* 作业 ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
                            /* 状态：UNDEF、BG、FG 或 ST */
    char cmdline[MAXLINE];  /* command line */
                            /* 命令行 */
};
struct job_t jobs[MAXJOBS]; /* The job list */
                            /* 作业列表 */
/* End global variables */
/* 全局变量结束 */


/* Function prototypes */
/* 函数原型 */

/* Here are the functions that you will implement */
/* 以下是你需要实现的函数 */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
/* 以下是我们为你提供的辅助例程 */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 * main - shell 的主例程
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */
                         /* 是否打印提示符（默认打印） */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    /* 把 stderr 重定向到 stdout（这样测试驱动就能从连接到 stdout 的管道
     * 拿到全部输出） */
    dup2(1, 2);

    /* Parse the command line */
    /* 解析命令行 */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
                              /* 打印帮助信息 */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
                              /* 输出额外诊断信息 */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
                              /* 不打印提示符 */
            emit_prompt = 0;  /* handy for automatic testing */
                              /* 便于自动测试 */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */
    /* 安装信号处理函数 */

    /* These are the ones you will need to implement */
    /* 以下是你需要实现的信号处理函数 */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
                                       /* 子进程终止或停止 */

    /* This one provides a clean way to kill the shell */
    /* 这个信号提供了一种干净地终止 shell 的方式 */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    /* 初始化作业列表 */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    /* 执行 shell 的读入/求值循环 */
    while (1) {

	/* Read command line */
	/* 读取命令行 */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	                   /* 文件结束（ctrl-d） */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	/* 求值命令行 */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
             /* 控制流不会到达这里 */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * eval - 求值用户刚刚输入的命令行
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
 *
 * 如果用户请求的是内建命令（quit、jobs、bg 或 fg），则立即执行。
 * 否则 fork 一个子进程，在子进程上下文中运行该作业。
 * 如果作业在前台运行，则等待它终止后再返回。注意：
 * 每个子进程都必须有唯一的进程组 ID，这样当我们在键盘上键入
 * ctrl-c（ctrl-z）时，内核发出的 SIGINT（SIGTSTP）就不会被后台子进程收到。
*/
void eval(char *cmdline) 
{
    char *argv[MAXARGS];
    int bg = parseline(cmdline, argv);
    if (argv[0] == NULL)
        return;
    if (builtin_cmd(argv))
        return;
    pid_t pid = fork();
    if (pid < 0) {
        unix_error("fork error");
    }
    if (pid == 0) {
        /* 子进程：exec */
        if (execve(argv[0], argv, environ) < 0) {
            printf("%s: Command not found.\n", argv[0]);
            exit(0);   /* 退出的是子进程，shell 还在 */
        }
    } else {
        /* 父进程：这里才能用 pid 去等这个孩子 */
        if (!bg) {
            addjob(jobs, pid, FG, cmdline);
            int status;
            waitpid(pid, &status, 0);
            if(WIFSIGNALED(status)){
                printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, WTERMSIG(status));
            }
            deletejob(jobs, pid);
        } else {
            addjob(jobs, pid, BG, cmdline);
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
        }
    }
    return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * parseline - 解析命令行并构造 argv 数组。
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 *
 * 单引号括起来的字符被视为一个参数。
 * 若用户请求后台作业则返回 true，若请求前台作业则返回 false。
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
                                /* 保存命令行的本地副本 */
    char *buf = array;          /* ptr that traverses command line */
                                /* 遍历命令行的指针 */
    char *delim;                /* points to first space delimiter */
                                /* 指向第一个空格分隔符 */
    int argc;                   /* number of args */
                                /* 参数个数 */
    int bg;                     /* background job? */
                                /* 是否为后台作业？ */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
                               /* 把末尾的 '\n' 替换成空格 */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
                                  /* 忽略前导空格 */
	buf++;

    /* Build the argv list */
    /* 构造 argv 列表 */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	                              /* 忽略空格 */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
                    /* 忽略空行 */
	return 1;

    /* should the job run in the background? */
    /* 该作业是否应在后台运行？ */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 * builtin_cmd - 如果用户输入的是内建命令，则立即执行。
 */
int builtin_cmd(char **argv) 
{
    if (strcmp(argv[0],"quit")==0){
        /* 结束整个 shell */
        exit(0);
    }
    if (strcmp(argv[0],"jobs")==0){
        /* 打印作业列表 return 1 */
        listjobs(jobs);
        return 1;
    }
    /* bg /fg */
    return 0;     /* not a builtin command */
                  /* 不是内建命令 */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 * do_bgfg - 执行内建命令 bg 和 fg
 */
void do_bgfg(char **argv) 
{
    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 * waitfg - 阻塞直到进程 pid 不再是前台进程
 */
void waitfg(pid_t pid)
{
    return;
}

/*****************
 * Signal handlers
 * 信号处理函数
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 * sigchld_handler - 每当子作业终止（变成僵尸进程），或因收到
 *     SIGSTOP / SIGTSTP 而停止时，内核会向 shell 发送 SIGCHLD。
 *     该处理函数回收所有当前可回收的僵尸子进程，但不会等待
 *     其他仍在运行的子进程终止。
 */
void sigchld_handler(int sig) 
{
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 * sigint_handler - 每当用户在键盘上键入 ctrl-c 时，内核会向 shell
 *    发送 SIGINT。捕获该信号并转发给前台作业。
 */
void sigint_handler(int sig) 
{
    pid_t pid = fgpid(jobs);
    if (pid == 0) {
        return;
    }
    kill(pid, SIGINT);
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 * sigtstp_handler - 每当用户在键盘上键入 ctrl-z 时，内核会向 shell
 *     发送 SIGTSTP。捕获该信号，并向前台作业发送 SIGTSTP 以挂起它。
 */
void sigtstp_handler(int sig) 
{
    return;
}

/*********************
 * End signal handlers
 * 信号处理函数结束
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 * 操作作业列表的辅助例程
 **********************************************/

/* clearjob - Clear the entries in a job struct */
/* clearjob - 清空作业结构体中的各项 */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
/* initjobs - 初始化作业列表 */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
/* maxjid - 返回已分配的最大作业 ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
/* addjob - 向作业列表添加一个作业 */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
/* deletejob - 从作业列表中删除 PID 为 pid 的作业 */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
/* fgpid - 返回当前前台作业的 PID；若没有则返回 0 */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
/* getjobpid  - 按 PID 在作业列表中查找作业 */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
/* getjobjid  - 按 JID 在作业列表中查找作业 */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
/* pid2jid - 将进程 ID 映射为作业 ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
/* listjobs - 打印作业列表 */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 * 作业列表辅助例程结束
 ******************************/


/***********************
 * Other helper routines
 * 其他辅助例程
 ***********************/

/*
 * usage - print a help message
 * usage - 打印帮助信息
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 * unix_error - Unix 风格的错误处理例程
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 * app_error - 应用程序风格的错误处理例程
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 * Signal - sigaction 函数的包装
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
                                  /* 阻塞正在处理的这类信号 */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */
                                  /* 若可能则重启被中断的系统调用 */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 * sigquit_handler - 测试驱动程序可以通过向子 shell 发送 SIGQUIT
 *    来优雅地终止它。
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



