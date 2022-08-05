#!/bin/sh
# Install Zephyr dependencies for Ubuntu
# Usage: install_deps.sh

# Update OS
sudo apt update
sudo apt upgrade

# Install dependencies

# Download and execute the Kitware archive script to add the Kitware APT repository to your sources list
wget https://apt.kitware.com/kitware-archive.sh
sudo bash kitware-archive.sh
rm -f kitware-archive.sh

# Use apt to install the required dependencies:
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-venv python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev

# Print versions of main dependencies for verification against the minimum requirements
cmake --version
python3 --version
dtc --version
