sudo add-apt-repository universe ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install pkg-config g++-13 clang-format clang-tidy cmake git build-essential libwayland-dev libxkbcommon-dev xorg-dev libgl1-mesa-dev mesa-common-dev mesa-utils doxygen graphviz

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 10
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 10
sudo update-alternatives --set cc /usr/bin/gcc
sudo update-alternatives --set c++ /usr/bin/g++

sudo update-alternatives --config gcc
sudo update-alternatives --config g++

glxinfo | grep OpenGL