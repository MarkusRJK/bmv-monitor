Install:

sudo apt install g++-aarch64-linux-gnu binutils-aarch64-linux-gnu \
                 libboost1.83-dev:arm64 \
                 libboost1.83-dev:amd64 # if you want to run via socat
                 libsdbus-c++-dev


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


Forwarding ttyUSB1 to local machine saves copying program over:

compile without Raspi.cmake (for local machine).

Run on homecontrol (as service):

sudo socat tcp-l:54321,reuseaddr,fork file:/dev/ttyUSB1,nonblock,waitlock=/var/run/tty.lock

Run on build machine:

mkdir $HOME/dev
socat pty,link=$HOME/dev/ttyV0,waitslave tcp:192.168.29.31:54321
