#!/bin/bash
set -eux

mkdir -p data
cd data
wget -O orb_vocab.dbow2 'https://github.com/SpectacularAI/SLAM-module/releases/download/orb_vocab/orb_vocab.dbow2'
