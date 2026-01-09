cd ~/Coinche
git pull origin feature/android-support
cd build
make server -j$(nproc)
sudo systemctl restart coinche-server

