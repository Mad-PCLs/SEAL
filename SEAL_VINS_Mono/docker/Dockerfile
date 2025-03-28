FROM ubuntu:16.04

ENV CERES_VERSION="1.14.0"
ENV ROS_DISTRO="kinetic"


# Install utilities
RUN apt-get update && apt-get install -y \
    wget \
    curl \
    gnupg \
    lsb-release \
    software-properties-common \
    cmake \
    build-essential \
    git \
    unzip

# Install ROS
RUN sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list' \
    && curl -sSL 'http://packages.ros.org/ros.key' | apt-key add - \
    && apt-get update && apt-get install -y \
    ros-kinetic-desktop-full \
    && rosdep init && rosdep update \
    && echo "source /opt/ros/kinetic/setup.bash" >> ~/.bashrc

# Install other VINS-Mono deps (Source: https://github.com/HKUST-Aerial-Robotics/VINS-Mono/blob/master/docker/Dockerfile)
RUN apt-get update && apt-get install -y \
      libatlas-base-dev \
      libeigen3-dev \
      libgoogle-glog-dev \
      libsuitesparse-dev \
      python-catkin-tools \
      ros-${ROS_DISTRO}-cv-bridge \
      ros-${ROS_DISTRO}-image-transport \
      ros-${ROS_DISTRO}-message-filters \
      ros-${ROS_DISTRO}-tf && \
      rm -rf /var/lib/apt/lists/* && \
      # Build and install Ceres
      git clone https://ceres-solver.googlesource.com/ceres-solver && \
      cd ceres-solver && \
      git checkout tags/${CERES_VERSION} && \
      mkdir build && cd build && \
      cmake .. && \
      make -j 4 install && \
      rm -rf ../../ceres-solver

# Install python tools you are usually using
RUN curl https://bootstrap.pypa.io/pip/3.5/get-pip.py -o get-pip.py \
    && python3.5 get-pip.py "pip==20.3.4" \
    && rm get-pip.py

RUN pip3 install --no-cache-dir numpy pandas matplotlib seaborn evo tabulate

WORKDIR /home
