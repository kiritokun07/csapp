本文件包含 attacklab 某一实验实例的材料。

文件说明：

    ctarget

带有代码注入漏洞的 Linux 二进制程序。用于作业的第 1–3 阶段。

    rtarget

带有面向返回编程（ROP）漏洞的 Linux 二进制程序。用于作业的第 4–5 阶段。

    cookie.txt

文本文件，包含本实验实例所需的 4 字节签名。

    farm.c

本实例 rtarget 中 gadget farm 的源代码。你可以编译它（使用 -Og 选项）并反汇编，以查找 gadgets。

    hex2raw

用于生成字节序列的工具程序。详见实验讲义中的文档。
