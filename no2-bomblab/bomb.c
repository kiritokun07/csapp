/***************************************************************************
 * Dr. Evil's Insidious Bomb, Version 1.1
 * 邪恶博士的阴险炸弹，版本 1.1
 * Copyright 2011, Dr. Evil Incorporated. All rights reserved.
 * 版权所有 2011，邪恶博士公司。保留所有权利。
 *
 * LICENSE: / 许可证：
 *
 * Dr. Evil Incorporated (the PERPETRATOR) hereby grants you (the
 * VICTIM) explicit permission to use this bomb (the BOMB).  This is a
 * time limited license, which expires on the death of the VICTIM.
 * 邪恶博士公司（加害方）特此明确授权你（受害者）使用本炸弹（BOMB）。
 * 本许可证有期限，于受害者死亡时失效。
 *
 * The PERPETRATOR takes no responsibility for damage, frustration,
 * insanity, bug-eyes, carpal-tunnel syndrome, loss of sleep, or other
 * harm to the VICTIM.  Unless the PERPETRATOR wants to take credit,
 * that is.
 * 加害方对受害者遭受的损害、挫败、精神错乱、眼球凸出、腕管综合征、
 * 失眠或其他伤害概不负责。除非加害方想邀功，那另当别论。
 *
 * The VICTIM may not distribute this bomb source code to
 * any enemies of the PERPETRATOR.  No VICTIM may debug,
 * reverse-engineer, run "strings" on, decompile, decrypt, or use any
 * other technique to gain knowledge of and defuse the BOMB.  BOMB
 * proof clothing may not be worn when handling this program.
 * 受害者不得将本炸弹源码分发给加害方的任何敌人。任何受害者不得调试、
 * 逆向工程、对炸弹运行 "strings"、反编译、解密，或使用任何其他技术
 * 来了解并拆除本炸弹。处理本程序时不得穿戴防爆服。
 *
 * The PERPETRATOR will not apologize for the PERPETRATOR's poor sense of
 * humor.  This license is null and void where the BOMB is prohibited
 * by law.
 * 加害方不会为自己糟糕的幽默感道歉。在法律禁止炸弹的地方，本许可证无效。
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "support.h"
#include "phases.h"

/* 
 * Note to self: Remember to erase this file so my victims will have no
 * idea what is going on, and so they will all blow up in a
 * spectaculary fiendish explosion. -- Dr. Evil
 * 备忘：记得删掉这个文件，这样受害者就不知道发生了什么，
 * 然后他们都会在一场壮观而邪恶的爆炸中完蛋。—— 邪恶博士
 */

FILE *infile;

int main(int argc, char *argv[])
{
    char *input;

    /* Note to self: remember to port this bomb to Windows and put a 
     * fantastic GUI on it.
     * 备忘：记得把这炸弹移植到 Windows，再加个炫酷的图形界面。 */

    /* When run with no arguments, the bomb reads its input lines 
     * from standard input.
     * 无命令行参数运行时，炸弹从标准输入读取各行输入。 */
    if (argc == 1) {  
	infile = stdin;
    } 

    /* When run with one argument <file>, the bomb reads from <file> 
     * until EOF, and then switches to standard input. Thus, as you 
     * defuse each phase, you can add its defusing string to <file> and
     * avoid having to retype it.
     * 带一个参数 <file> 运行时，炸弹先从该文件读到 EOF，再切换到标准输入。
     * 因此每拆除一关，可以把通关字符串追加进 <file>，就不用每次重打。 */
    else if (argc == 2) {
	if (!(infile = fopen(argv[1], "r"))) {
	    printf("%s: Error: Couldn't open %s\n", argv[0], argv[1]);
	    /* 错误：无法打开文件 */
	    exit(8);
	}
    }

    /* You can't call the bomb with more than 1 command line argument.
     * 不能带超过 1 个命令行参数调用炸弹。 */
    else {
	printf("Usage: %s [<input_file>]\n", argv[0]);
	/* 用法：程序名 [<输入文件>] */
	exit(8);
    }

    /* Do all sorts of secret stuff that makes the bomb harder to defuse.
     * 做各种机密初始化，让炸弹更难拆除。 */
    initialize_bomb();

    printf("Welcome to my fiendish little bomb. You have 6 phases with\n");
    printf("which to blow yourself up. Have a nice day!\n");
    /* 欢迎来到我邪恶的小炸弹。你有 6 个关卡可以把自己炸飞。祝你今天愉快！ */

    /* Hmm...  Six phases must be more secure than one phase!
     * 嗯……六关肯定比一关更安全！ */
    input = read_line();             /* Get input / 获取输入          */
    phase_1(input);                  /* Run the phase / 运行本关      */
    phase_defused();                 /* Drat!  They figured it out!
				      * Let me know how they did it.
				      * 可恶！他们弄明白了！
				      * 告诉我他们是怎么做到的。 */
    printf("Phase 1 defused. How about the next one?\n");
    /* 第 1 关已拆除。下一关怎么样？ */

    /* The second phase is harder.  No one will ever figure out
     * how to defuse this...
     * 第二关更难。没人能弄清怎么拆除这一关…… */
    input = read_line();
    phase_2(input);
    phase_defused();
    printf("That's number 2.  Keep going!\n");
    /* 这是第 2 关。继续！ */

    /* I guess this is too easy so far.  Some more complex code will
     * confuse people.
     * 到目前为止好像太简单了。再来点更复杂的代码迷惑他们。 */
    input = read_line();
    phase_3(input);
    phase_defused();
    printf("Halfway there!\n");
    /* 已经过半了！ */

    /* Oh yeah?  Well, how good is your math?  Try on this saucy problem!
     * 哦是吗？那你的数学怎么样？试试这道调皮的题！ */
    input = read_line();
    phase_4(input);
    phase_defused();
    printf("So you got that one.  Try this one.\n");
    /* 这关你过了。试试这一关。 */
    
    /* Round and 'round in memory we go, where we stop, the bomb blows!
     * 在内存里转啊转，停在哪儿，炸弹就炸！ */
    input = read_line();
    phase_5(input);
    phase_defused();
    printf("Good work!  On to the next...\n");
    /* 干得好！下一关…… */

    /* This phase will never be used, since no one will get past the
     * earlier ones.  But just in case, make this one extra hard.
     * 这一关永远用不上，因为没人能过前面几关。但以防万一，把它做得特别难。 */
    input = read_line();
    phase_6(input);
    phase_defused();

    /* Wow, they got it!  But isn't something... missing?  Perhaps
     * something they overlooked?  Mua ha ha ha ha!
     * 哇，他们居然过了！不过是不是少了点什么……？也许他们漏掉了什么？
     * 姆哈哈哈哈！ */
    
    return 0;
}
