# CS:APP Cache Lab 实验说明

这是 CS:APP Cache Lab 的发放目录（handout）。

## 运行自动评分程序

在运行自动评分程序之前，先编译代码：

```bash
linux> make
```

检查缓存模拟器的正确性：

```bash
linux> ./test-csim
```

检查转置函数的正确性与性能：

```bash
linux> ./test-trans -M 32 -N 32
linux> ./test-trans -M 64 -N 64
linux> ./test-trans -M 61 -N 67
```

一次性检查全部内容（这就是助教/老师实际运行的程序）：

```bash
linux> ./driver.py
```

## 文件说明

需要修改并提交的两个文件：

| 文件 | 说明 |
|------|------|
| `csim.c` | 你的缓存模拟器 |
| `trans.c` | 你的矩阵转置函数 |

用于评测模拟器和转置函数的工具：

| 文件 | 说明 |
|------|------|
| `Makefile` | 构建模拟器与相关工具 |
| `README` | 本说明文件 |
| `driver.py*` | 驱动程序，会运行 `test-csim` 和 `test-trans` |
| `cachelab.c` | 必需的辅助函数 |
| `cachelab.h` | 必需的头文件 |
| `csim-ref*` | 参考版缓存模拟器可执行文件 |
| `test-csim*` | 测试你的缓存模拟器 |
| `test-trans.c` | 测试你的转置函数 |
| `tracegen.c` | `test-trans` 使用的辅助程序 |
| `traces/` | `test-csim` 使用的 trace 文件 |
