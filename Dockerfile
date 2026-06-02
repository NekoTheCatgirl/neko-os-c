FROM alpine:latest
LABEL authors="neko"

RUN apk add --no-cache \
    xorriso \
    grub \
    grub-bios \
    grub-efi \
    nasm \
    gcc \
    g++ \
    make \
    cmake \
    mtools \
    wget \
    tar \
    gmp-dev \
    mpfr-dev \
    mpc1-dev \
    zlib-dev \
    texinfo

# Build binutils cross-compiler for x86_64-elf
RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz && \
    tar -xf binutils-2.42.tar.gz && \
    mkdir binutils-build && \
    cd binutils-build && \
    ../binutils-2.42/configure \
        --target=x86_64-elf \
        --prefix=/usr/local \
        --disable-nls \
        --disable-werror && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf binutils-2.42 binutils-2.42.tar.gz binutils-build

# Build gcc cross-compiler for x86_64-elf
RUN wget https://ftp.gnu.org/gnu/gcc/gcc-16.1.0/gcc-16.1.0.tar.gz && \
    tar -xf gcc-16.1.0.tar.gz && \
    cd gcc-16.1.0 && \
    contrib/download_prerequisites && \
    cd / && mkdir gcc-build && cd gcc-build && \
    ../gcc-16.1.0/configure \
        --target=x86_64-elf \
        --prefix=/usr/local \
        --disable-nls \
        --disable-werror \
        --without-headers \
        --enable-languages=c,c++ \
        --disable-hosted-libstdc++ && \
    make -j$(nproc) all-gcc && \
    make -j$(nproc) all-target-libgcc && \
    make install-gcc && \
    make install-target-libgcc && \
    cd / && rm -rf gcc-16.1.0 gcc-16.1.0.tar.gz gcc-build

RUN apk add --no-cache \
    xorriso \
    grub \
    grub-bios \
    grub-efi \
    nasm \
    gcc \
    g++ \
    make \
    cmake \
    mtools