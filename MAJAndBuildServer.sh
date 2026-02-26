cd ~/Coinche
git pull origin main
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make server -j$(nproc)
sudo systemctl restart coinche-server