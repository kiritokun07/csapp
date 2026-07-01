# CSAPP 实验环境：Linux x86-64，支持 Data Lab (-m32) 与后续实验
FROM --platform=linux/amd64 ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gcc-multilib \
    gdb \
    perl \
    valgrind \
    file \
    vim-tiny \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /csapp
