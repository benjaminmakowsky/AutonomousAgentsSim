#! /bin/bash
sudo mv share/xpilot-ng/shipshapes.txt share/xpilot-ng/shipshapes2.txt
sudo ./configure --prefix=$PWD --silent
sudo make CC='gcc -fPIC' --silent
sudo make install --silent
sudo mv share/xpilot-ng/shipshapes2.txt share/xpilot-ng/shipshapes.txt
cd ../xpilot_bitmaps/
./install_bitmaps.sh
cd ../xpilot-engine
