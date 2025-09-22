new:

build:
cmake .
make

the run with 19200 and dev/serial/by-id/usb-VictronEnergy_BV_VE_Direct_cable_VE1SUT80-if00-port0

gcc -pthread main.cpp
g++ main-boost-reader.cpp -o miniboost -pthread -lboost_system -lboost_thread

run with 19200 baud and /dev/serial/by-id/usb-Vic...



Steps:

1. put /etc/apt/sources.list.d/arm-port.list containing refs to ports.ubuntu.com
2. dpkg --add-architecture armhf
3. install e.g. apt install libboost-system1.83-dev:armhf

See contents with dpkg -L for setting up lib/incl for compilation.

This way e.g. boost ist installed with version 1.83.0 for armhf and amd64!!!
So they have the same includes!