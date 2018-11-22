#!/bin/sh

cd /home/pi/backend
git fetch origin
git reset --hard origin/master
git checkout -f
git pull origin master

npm install
