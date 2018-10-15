# update embedded
sudo pkill -f movit
cd ~/embedded
git checkout -f
git pull origin master

# update front-end
sudo kill $(lsof -t -i:3000)
cd ~/frontend
git checkout -f
git pull origin master

# update back-end
node-red-stop
sudo pkill -f node-red
cd ~/backend
git checkout -f
git pull origin master

# start all
cd ~/embedded
./start_backend.sh & 
./start_frontend.sh & 
./start_embedded.sh & 