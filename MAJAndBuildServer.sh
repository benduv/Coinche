cd ~/Coinche
git pull origin feature/android-support
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make server -j$(nproc)
sudo systemctl restart coinche-server

