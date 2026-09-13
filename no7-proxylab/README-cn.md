####################################################################
# CS:APP Proxy Lab
# 发给学生的实验材料
####################################################################

本目录包含完成 CS:APP Proxy Lab 所需的文件。

proxy.c
csapp.h
csapp.c
    这些是起步文件。csapp.c 和 csapp.h 在教材中有说明。

    你可以任意修改这些文件，也可以自行创建并提交额外文件。

    请使用 `port-for-user.pl` 或 `free-port.sh` 为你的 proxy
    或 tiny 服务器生成互不冲突的端口。

Makefile
    用来编译 proxy 程序。输入 "make" 即可编译你的实现；
    或者先 "make clean" 再 "make"，做一次干净编译。

    输入 "make handin" 会生成你要提交的 tar 包。
    你可以按自己的需要修改 Makefile。助教会用你的
    Makefile 从源码编译你的 proxy。

port-for-user.pl
    为特定用户生成一个随机端口。
    用法：./port-for-user.pl <userID>

free-port.sh
    查找一个当前未被占用的 TCP 端口，供你的 proxy 或 tiny 使用。
    用法：./free-port.sh

driver.sh
    自动评分脚本，覆盖 Basic、Concurrency 和 Cache 三部分。
    用法：./driver.sh

nop-server.py
    自动评分脚本用到的辅助程序。

tiny
    教材中的 Tiny Web 服务器。
