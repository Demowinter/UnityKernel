FROM debian:stable

ENV DEBIAN_FRONTEND=noninteractive

RUN dpkg --add-architecture amd64 \
    && apt-get update && apt-get install -y \
    gcc-i686-linux-gnu \
    g++-i686-linux-gnu \
    binutils-i686-linux-gnu \
    cmake \
    make \
    linux-libc-dev \
    nasm \
    mtools \
    xorriso \
    grub-pc-bin:amd64 \
    grub-common \
    qemu-system-x86 \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=i686-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=i686-linux-gnu-g++ \
    -DCMAKE_ASM_COMPILER=i686-linux-gnu-gcc \
    && cmake --build build --target iso --parallel

CMD ["bash"]