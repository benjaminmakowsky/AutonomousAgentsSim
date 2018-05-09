#! /bin/bash
mv share/xpilot-ng/shipshapes.txt share/xpilot-ng/shipshapes2.txt
./configure --prefix=$PWD --silent
make CC='gcc -fPIC' --silent
make install --silent
mv share/xpilot-ng/shipshapes2.txt share/xpilot-ng/shipshapes.txt
cd ../xpilot_bitmaps/
./install_bitmaps.sh
cd ../xpilot-engine
