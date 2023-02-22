FROM ubuntu:18.04

USER root

RUN apt-get update --fix-missing && \
    apt-get install -y \
        build-essential \
        file \
        sudo \
        apt-utils \
        cmake \
        cmake-curses-gui \
        mingw-w64 \
        libsdl2-dev \
        libsdl2-image-dev \
        libopenal-dev \
        libalure-dev \
        libenet-dev \
        libfreetype6-dev \
        libglew-dev \
        libglm-dev \
        libaudio-dev \
        libswscale-dev

# Passwordless sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

RUN useradd -ms /bin/bash dev && usermod -aG sudo dev
USER dev
WORKDIR /home/dev
