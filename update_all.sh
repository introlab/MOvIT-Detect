#!/bin/sh
cd /home/pi/embedded
./kill_all.sh

./update_frontend.sh
./update_backend.sh
./update_embedded.sh
./start_all.sh
