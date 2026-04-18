#!/bin/bash
set -e

OS="$(uname -s)"

if [ "$OS" = "Darwin" ]; then
    if ! command -v brew &> /dev/null; then
        echo "Homebrew is required. Install it from https://brew.sh"
        exit 1
    fi

    brew install \
        pkg-config \
        cmake \
        git \
        llvm \
        clang-format \
        doxygen \
        graphviz \
        python3 \
        ninja \
        assimp

    python3 -m venv venv
    source "venv/bin/activate"
    pip3 install --upgrade pip
    pip3 install libclang

    echo "macOS setup complete ✅"

elif [ "$OS" = "Linux" ]; then
    sudo apt update
    sudo apt install -y software-properties-common
    sudo add-apt-repository -y universe
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo apt update

    sudo apt install -y \
        pkg-config \
        g++-13 \
        gcc-13 \
        clang-format \
        clang-tidy \
        cmake \
        git \
        build-essential \
        libwayland-dev \
        libxkbcommon-dev \
        xorg-dev \
        libgl1-mesa-dev \
        mesa-common-dev \
        mesa-utils \
        doxygen \
        graphviz \
        libassimp-dev \
        python3-pip \
        python3-venv

    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 10
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 10
    sudo update-alternatives --set gcc /usr/bin/gcc-13
    sudo update-alternatives --set g++ /usr/bin/g++-13

    python3 -m venv venv
    source "venv/bin/activate"
    pip3 install --upgrade pip
    pip3 install libclang

    if command -v glxinfo &> /dev/null; then
        glxinfo | grep "OpenGL"
    fi

    glxinfo | grep OpenGL
    echo "Linux setup complete ✅"
else
    echo "Unsupported OS: $OS"
    exit 1
fi