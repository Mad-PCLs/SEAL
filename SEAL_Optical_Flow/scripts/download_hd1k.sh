#!/bin/bash

mkdir -p ../data

if ! [ -d ../data/hd1k_input ]; then
    # Download the input images
    if ! [ -f ../data/hd1k_input.zip ]; then
        wget -O ../data/hd1k_input.zip http://hci-benchmark.iwr.uni-heidelberg.de/media/downloads/hd1k_input.zip
    fi

    unzip ../data/hd1k_input.zip -d ../data/
fi

# Download the ground truth
if ! [ -d ../data/hd1k_flow_gt ]; then
    # Download the input images
    if ! [ -f ../data/hd1k_flow_gt.zip ]; then
        wget -O ../data/hd1k_flow_gt.zip http://hci-benchmark.iwr.uni-heidelberg.de/media/downloads/hd1k_flow_gt.zip
    fi

    unzip ../data/hd1k_flow_gt.zip -d ../data/
fi

# Download the uncertainties
if ! [ -d ../data/hd1k_flow_unc ]; then
    # Download the input images
    if ! [ -f ../data/hd1k_flow_unc.zip ]; then
        wget -O ../data/hd1k_flow_unc.zip http://hci-benchmark.iwr.uni-heidelberg.de/media/downloads/hd1k_flow_uncertainty.zip
    fi

    unzip ../data/hd1k_flow_unc.zip -d ../data/
    mv ../data/hd1k_flow_uncertainty ../data/hd1k_flow_unc
fi
