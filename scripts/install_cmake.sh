#!/bin/bash
set -e
sudo apt update
sudo apt install -y software-properties-common apt-transport-https ca-certificates wget
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt update
sudo apt install -y cmake
cmake --version
