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

cd ~/embedded

./kill_all.sh

# start all
./start_backend.sh &
./start_frontend.sh &
./start_embedded.sh &