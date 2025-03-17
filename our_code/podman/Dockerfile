# old debian is needed to get cpython 2.7
FROM debian:12

MAINTAINER dacin21@users.noreply.github.com

# general linux packages
RUN apt-get update
RUN apt install tmux neovim time -y

# c++ toolchain
RUN apt install build-essential -y

# meson build system
RUN apt install cmake pkg-config -y
RUN apt install python3 python3-pip meson -y

# c++ libraries
RUN apt install libboost-all-dev libcgal-dev libgtest-dev  -y


CMD ["/bin/bash"]

