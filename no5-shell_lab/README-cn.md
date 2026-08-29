# CS:APP Shell Lab 实验说明

这是 CS:APP Shell Lab 的发放目录。

## 文件说明

| 文件 | 说明 |
|------|------|
| `Makefile` | 编译你的 shell 程序并运行测试 |
| `README` | 本说明文件 |
| `tsh.c` | 你需要编写并提交的 shell 程序 |
| `tshref` | 参考 shell 的二进制文件 |

其余文件用于测试你的 shell：

| 文件 | 说明 |
|------|------|
| `sdriver.pl` | 基于 trace 驱动的 shell 测试程序 |
| `trace*.txt` | 控制测试程序的 15 个 trace 文件 |
| `tshref.out` | 参考 shell 在全部 15 个 trace 上的示例输出 |

trace 文件会调用的小型 C 程序：

| 文件 | 说明 |
|------|------|
| `myspin.c` | 接收参数 `<n>`，空转 `<n>` 秒 |
| `mysplit.c` | fork 出一个子进程，空转 `<n>` 秒 |
| `mystop.c` | 空转 `<n>` 秒后向自身发送 `SIGTSTP` |
| `myint.c` | 空转 `<n>` 秒后向自身发送 `SIGINT` |
