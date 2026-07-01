#!/usr/bin/env bash
# 在 Linux x86-64 容器中运行 CSAPP 实验命令（适用于 Apple Silicon Mac）
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
IMAGE="csapp:amd64"
CONTAINER="csapp-lab-dev"

usage() {
  cat <<'EOF'
用法:
  ./lab.sh build              构建实验 Docker 镜像
  ./lab.sh shell              进入交互式容器 shell
  ./lab.sh datalab [cmd...]   在 no1-datalab 中执行命令（默认 make btest）
  ./lab.sh <任意命令>         在容器内于仓库根目录执行

示例:
  ./lab.sh build
  ./lab.sh datalab make btest
  ./lab.sh datalab ./btest -f bitXor
  ./lab.sh datalab make && ./lab.sh datalab ./ishow 0x27
EOF
}

ensure_image() {
  if ! docker image inspect "$IMAGE" &>/dev/null; then
    echo "镜像 $IMAGE 不存在，正在构建..."
    docker compose build csapp
  fi
}

run_in_container() {
  ensure_image
  if [ -t 0 ] && [ -t 1 ]; then
    docker run --rm -it \
      --platform linux/amd64 \
      -v "$ROOT:/csapp" \
      -w /csapp \
      "$IMAGE" \
      "$@"
  else
    docker run --rm \
      --platform linux/amd64 \
      -v "$ROOT:/csapp" \
      -w /csapp \
      "$IMAGE" \
      "$@"
  fi
}

case "${1:-shell}" in
  -h|--help|help)
    usage
    ;;
  build)
    docker compose build csapp
    echo "构建完成: $IMAGE"
    ;;
  shell)
    run_in_container bash
    ;;
  datalab)
    shift
    if [ $# -eq 0 ]; then
      set -- make btest
    fi
    run_in_container bash -lc "cd no1-datalab && $*"
    ;;
  *)
    run_in_container "$@"
    ;;
esac
