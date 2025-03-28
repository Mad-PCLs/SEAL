#!/bin/bash

# Get the path of the parent folder of the directory of this script,
# in order to then mount it inside the docker container for persistence.
## Source: https://stackoverflow.com/a/20196098
HOST_CODE="$(dirname "$(dirname "$(dirname "$(readlink -fm "$0")")")")"

# Get just the name of the above directory, so you use the same folder
# name inside the docker container.
## Source: https://stackoverflow.com/a/5257398
parent_foldername=(${HOST_CODE//// })
parent_foldername=${parent_foldername[-1]}

# Define the path of the folder of the parent container.
MOUNT_CODE=/home/${parent_foldername}

# Replace line 10830 at:
# vio_benchmark/hybvio_runner/HybVIO/3rdparty/mobile-cv-suite/accelerated-arrays\
# /test/Catch2/single_include/catch2/catch.hpp
sed -i '10830s|.*|    static constexpr std::size_t sigStackSize = 32768;|' ../HybVIO/3rdparty/mobile-cv-suite/accelerated-arrays/test/Catch2/single_include/catch2/catch.hpp

if [[ "$1" == "build" ]]; then

    # Build docker image
    docker build \
        -t hybvio:latest .

elif [[ "$1" == "run" ]]; then
    # Start docker container deamon
    docker run -it --privileged --net=host \
        -v $HOST_CODE:$MOUNT_CODE \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -e DISPLAY=$DISPLAY \
        -h $HOSTNAME \
        -v $HOME/.Xauthority:/home/.Xauthority \
        -e XAUTHORITY=/home/.Xauthority \
        --name hybvio_container \
        hybvio:latest

else
    echo "Usage: ./docker_helper.sh [build|run]"
fi
