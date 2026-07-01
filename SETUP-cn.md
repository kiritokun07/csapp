# CSAPP 实验环境配置（macOS / Apple Silicon）

本仓库实验需在 **Linux x86-64** 下运行。Apple Silicon Mac 无法直接 `make`（Data Lab 使用 `-m32` 32 位编译）。

## 前置条件

- [Docker](https://docs.docker.com/get-docker/) 或 [OrbStack](https://orbstack.dev/)（已检测到可用）

## 快速开始（推荐 docker compose）

```bash
# 1. 构建镜像（首次约 1–2 分钟）
docker compose build

# 2. Data Lab：编译并运行全部测试
docker compose run --rm datalab-test

# 3. 测试单个函数
docker compose run --rm -T csapp bash -lc "cd no1-datalab && make btest && ./btest -f bitXor"

# 4. 进入交互式容器
docker compose run --rm csapp bash
```

也可使用 `./lab.sh`（效果相同）：

```bash
chmod +x lab.sh
./lab.sh datalab make btest
./lab.sh datalab ./btest
./lab.sh shell
```

## 各实验说明

### Data Lab (`no1-datalab/`)

| 工具 | 作用 |
|------|------|
| `make btest` | 编译测试程序 |
| `./btest` | 验证 `bits.c` 正确性 |
| `./ishow <数>` | 查看整数位表示 |
| `./fshow <数>` | 查看浮点位表示 |

**关于 `dlc`**：规则检查器 `dlc` 是 Linux 预编译二进制，本仓库未包含。可用 `btest` 验证正确性；若需检查运算符数量与编码规范，需自行从课程资料获取 `dlc` 放入 `no1-datalab/`。

### Bomb Lab (`no2-bomblab/`)

当前仅有 `bomb.c` 源码框架，完整实验还需要 `bomb` 可执行文件及 `phases.h`、`support.c` 等（通常由课程发放）。环境已预装 `gdb`、`objdump`，拿到二进制后可在容器内：

```bash
./lab.sh shell
cd no2-bomblab
gdb ./bomb
```

### 后续实验（Cache / Malloc / Shell / Proxy 等）

容器内已安装 `gcc`、`make`、`gdb`、`valgrind`，添加对应 lab 目录后可直接在容器中编译运行。

## 常见问题

**Q: 为什么不用本机 `gcc`？**  
A: Data Lab 的 Makefile 使用 `-m32`，在 arm64 macOS 上链接会失败。

**Q: 容器很慢？**  
A: 使用 `linux/amd64` 镜像在 Apple Silicon 上通过模拟运行，属正常现象；日常编辑在宿主机，仅编译/测试进容器即可。

**Q: 每次都要 `./lab.sh` 吗？**  
A: 可 `./lab.sh shell` 进入容器后连续操作；退出容器后改动会保留在挂载目录中。
