#!/bin/sh
# This script is executed by rc.local at startup

cd /home/pi/embedded

./kill_all.sh

# start all
./start_control.sh &
./start_frontend.sh &