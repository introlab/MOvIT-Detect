echo "Stopping node-red"
node-red-stop
echo "Killing node-red"
sudo killall -w node-red
echo "Killing movit"
sudo pkill -f movit
echo "Killing npm"
sudo killall -w npm
echo "Killing node"
sudo killall -w node
sudo pkill -f node
echo "Killing yarn"
sudo killall -w yarn