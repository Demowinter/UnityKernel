FROM debian:stable

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    g++-multilib \
    cmake \
    make \
    binutils \
    libc6-dev \
    libc6-dev-i386 \
    linux-libc-dev \
    nasm \
    mtools\
    xorriso \
    grub-pc-bin \
    grub-common \
    qemu-system-x86 \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build --target iso --parallel

CMD ["bash"]