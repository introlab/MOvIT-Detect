# update embedded
# sudo pkill -f movit
# cd ~/embedded
# git checkout -f
# git pull origin master

# # update front-end
# cd ~/frontend
# git checkout -f
# git pull origin master

# # update back-end
# node-red-stop
# sudo pkill -f node-red
# cd ~/backend
# git checkout -f
# git pull origin master

cd /home/pi/embedded
sudo chmod +x *.sh

./kill_all.sh

# start all
./start_control.sh &
./start_frontend.sh &