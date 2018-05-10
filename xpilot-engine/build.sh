#! /bin/bash
./configure --prefix=$PWD --silent
make CC='gcc -fPIC' --silent
make install --silent
cd ../xpilot_bitmaps/
./install.sh
cd ../xpilot-engine
