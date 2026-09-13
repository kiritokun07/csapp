#####################################################################
# CS:APP Malloc Lab
# 发给学生的实验材料
#
# Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
# 未经许可不得使用、修改或复制。
#
######################################################################

***********
主要文件：
***********

mm.{c,h}
	你要实现的 malloc 软件包。mm.c 是你需要提交的文件，
	也是你唯一应该修改的文件。

mdriver.c
	用来测试你的 mm.c 的 malloc 驱动程序

short{1,2}-bal.rep
	两份很小的 trace 文件，方便你上手调试。

Makefile
	用来编译驱动程序

**********************************
驱动程序的其他支撑文件
**********************************

config.h	配置 malloc lab 驱动程序
fsecs.{c,h}	对不同计时包的封装函数
clock.{c,h}	访问 Pentium 和 Alpha 周期计数器的例程
fcyc.{c,h}	基于周期计数器的计时函数
ftimer.{c,h}	基于 interval timer 和 gettimeofday() 的计时函数
memlib.{c,h}	模拟堆以及 sbrk 函数

*******************************
编译并运行驱动程序
*******************************
在 shell 中输入 "make" 即可编译驱动程序。

用一份很小的测试 trace 运行驱动程序：

	unix> mdriver -V -f short1-bal.rep

-V 选项会打印有用的跟踪信息和汇总信息。

查看驱动程序的全部命令行选项：

	unix> mdriver -h
