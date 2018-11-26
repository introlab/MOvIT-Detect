#!/bin/sh

cd /home/pi/frontend
git fetch origin
git reset --hard origin/master
git checkout -f
git pull origin master

yarn install
