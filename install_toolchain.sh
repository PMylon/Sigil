#!/bin/sh
# Installs the Zephyr SDK which contains the toolchains for all supported architectures

# Download the SDK in the home directory
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.13.2/zephyr-sdk-0.13.2-linux-x86_64-setup.run

# Run the installer
chmod +x zephyr-sdk-0.13.2-linux-x86_64-setup.run
./zephyr-sdk-0.13.2-linux-x86_64-setup.run -- -d ~/zephyr-sdk-0.13.2

# Install udev rules, which allow you to flash most Zephyr boards as a regular user
sudo cp ~/zephyr-sdk-0.13.2/sysroots/x86_64-pokysdk-linux/usr/share/openocd/contrib/60-openocd.rules /etc/udev/rules.d
sudo service udev restart
sudo udevadm control --reload
