#!/bin/sh

cd /home/pi/embedded
git fetch origin
git reset --hard origin/master
git checkout -f
git clean -fdx
git pull origin master
