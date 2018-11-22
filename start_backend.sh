#!/bin/sh

cd /home/pi/backend

# execute server
node-red --userDir ./ --max-old-space-size=256
