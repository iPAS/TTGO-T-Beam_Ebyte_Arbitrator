#!/bin/bash

# https://stackoverflow.com/questions/62717698/how-to-create-a-dummy-pipe-pseudo-serial-device-on-linux
# This listens on UDP port 5000 on the loopback network interface.
# All data received is sent to the virtual serial device at /dev/ttyS0.
# All data received on the virtual serial device is sent to UDP address 127.0.0.1:5001.
# socat UDP:127.0.0.1:5001,bind=127.0.0.1:5000  \
#       PTY,link=/dev/ttyS0,raw,echo=0,waitslave


# https://github.com/ezramorris/PyVirtualSerialPorts
# virtualserialports 2
# $ minicom -D /dev/pts/0
# $ minicom -D /dev/pts/1
