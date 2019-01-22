#!/bin/sh

cd /home/pi/MOvIT-Detect-Backend

# execute server
node-red --userDir ./ --max-old-space-size=256

