#! /bin/bash
# configure
./configure --bindir=$PWD/../xpilot_bin --prefix=$PWD --silent LIBS='-lpthread -lsqlite3'

#base build
#make CC='gcc -lpthread -lsqlite3 -fPIC' --silent
make CC='gcc -fPIC' --silent
make install --silent

#install bitmaps & shipshapes
cd ../xpilot_bitmaps/
./install.sh
cd ../xpilot_engine

#move the binaries in bin into their respective folders
cd ../xpilot_bin
mv xpilot-ng-server server
mv xpilot-ng-x11 client
mv xpilot-ng-xp-mapedit map_editor

#clean-up the bin dir, remove the exe's that we won't use
rm xpilot-ng-replay
rm xpilot-ng-sdl

#go back to root dir
cd ../xpilot_engine
