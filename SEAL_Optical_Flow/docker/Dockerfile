FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gcc \
    g++ \
    python3 \
    python3-venv \
    python3-pip \
    libopencv-dev \
    wget \
    unzip

RUN pip install --no-cache-dir numpy pandas matplotlib

ENV PATH="/opt/venv/bin:$PATH"

CMD ["/bin/bash"]
