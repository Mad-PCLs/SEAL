FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Install tools for Ubuntu
RUN apt-get update && apt-get install -y \
    wget \
    curl \
    gnupg \
    lsb-release \
    software-properties-common \
    cmake \
    git \
    unzip \
    libopencv-dev

# Install libraries for Ubuntu
RUN apt-get update && apt-get install -y \
    libgstreamer1.0-dev \
    libglfw3 \
    libglfw3-dev \
    ffmpeg \
    clang \
    libgtk2.0-dev \
    libvtk9-dev \
    libavcodec-dev \
    libswscale-dev \
    libavformat-dev \
    libglfw3 \
    libglfw3-dev \
    libwayland-dev \
    libxkbcommon-dev \
    wayland-protocols \
    extra-cmake-modules

RUN apt-get update && apt-get install -y \
    python3-pip

# Install basic python packages required to run scripts
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir numpy pandas matplotlib seaborn openpyxl evo tabulate

WORKDIR /home
