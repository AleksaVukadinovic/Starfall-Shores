#!/bin/bash
set -e

OS="$(uname -s)"

if [ "$OS" = "Darwin" ]; then
    if ! command -v brew &> /dev/null; then
        echo "Homebrew is required. Install it from https://brew.sh"
        exit 1
    fi

    brew install pkg-config cmake git llvm clang-format doxygen graphviz python3 ninja

    python3 -m venv venv
    source "venv/bin/activate" && pip3 install libclang

elif [ "$OS" = "Linux" ]; then
    sudo add-apt-repository universe ppa:ubuntu-toolchain-r/test
    sudo apt update
    sudo apt install pkg-config g++-13 clang-format clang-tidy cmake git build-essential libwayland-dev libxkbcommon-dev xorg-dev libgl1-mesa-dev mesa-common-dev mesa-utils doxygen graphviz

    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 10
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 10
    sudo update-alternatives --set cc /usr/bin/gcc
    sudo update-alternatives --set c++ /usr/bin/g++

    sudo update-alternatives --config gcc
    sudo update-alternatives --config g++

    sudo apt install python3-pip python3-venv -y

    python3 -m venv venv
    source "venv/bin/activate" && pip3 install libclang

    glxinfo | grep OpenGL
else
    echo "Unsupported OS: $OS"
    exit 1
fi