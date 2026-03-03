cd ~/TestCoinche/Coinche
git pull origin v0.2.0
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make server -j$(nproc)
sudo systemctl restart test-coinche