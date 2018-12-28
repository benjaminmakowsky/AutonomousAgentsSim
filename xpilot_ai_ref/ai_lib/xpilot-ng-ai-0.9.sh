#! /bin/bash
# Evan Gray - 21 May 2012
# Install dependencies
sudo apt-get -y install libsdl1.2debian libsdl1.2-dev libsdl-ttf2.0-dev libsdl-image1.2-dev libexpat1-dev zlib1g-dev gcc make
# Download and compile XPilot NG
mkdir xpilot-ai-temp
wget http://xpilot-ai.org/downloads/xpilot-ng-4.7.3.tar.gz
tar xfz xpilot-ng-4.7.3.tar.gz
rm xpilot-ng-4.7.3.tar.gz
mv xpilot-ng-4.7.3 xpilot-ng-ai
cd xpilot-ng-ai
echo;echo "Building and Installing XPilot. This may take a few minutes."
./configure --prefix=$PWD --silent
make CC='gcc -fPIC' --silent
make install --silent
# Install Xpilot-AI maps
wget http://xpilot-ai.org/downloads/maps.tar.gz
tar xfz maps.tar.gz
rm maps.tar.gz
mv maps/* share/xpilot-ng/maps
rm -r maps
# Relocate files
cp -r share ../xpilot-ai-temp/
mv src/server/xpilot-ng-server ../xpilot-ai-temp/
mv src/client/sdl/xpilot-ng-sdl ../xpilot-ai-temp/
mv src/client/x11/xpilot-ng-x11 ../xpilot-ai-temp/
# Download and compile AI code
cd src/client/x11
# Download bots
echo;echo -n "Would you like to install some example bots? Y/n --> "
read bots
echo;echo "What languages would you like to install?"
# Download and build C library
echo -n "C: Y/n --> "
read choice
if [[ "$choice" != "N" && "$choice" != "n" ]]; then
  wget http://xpilot-ai.org/downloads/xpilot-ng-ai-0.9/cAI.c
  wget http://xpilot-ai.org/downloads/xpilot-ng-ai-0.9/cAI.h
  gcc -w -fPIC -Wl,-wrap,Handle_end -DHAVE_CONFIG_H -I. -I../../..  -DCONF_DATADIR=\"../../../share/xpilot-ng/\" -I../../../src/common -I../../../src/client -g -O2 -MT cAI.o -MD -MP -MF .deps/cAI.Tpo -c -o cAI.o cAI.c
  gcc -o libcAI.so -fPIC -shared -Wl,-soname,libcAI.so,-wrap,Handle_end cAI.o about.o bitmaps.o colors.o configure.o dbuff.o guimap.o guiobjects.o join.o paintdata.o painthud.o paintradar.o record.o talk.o welcome.o widget.o xdefault.o xevent.o xeventhandlers.o xinit.o xpaint.o xpilot.o ../../../src/client/libxpclient.a ../../../src/common/libxpcommon.a   -lSM -lICE   -lX11  -lexpat -lz -lm
  mkdir ../../../../xpilot-ai-temp/c
  mv cAI.h ../../../../xpilot-ai-temp/c
  mv libcAI.so ../../../../xpilot-ai-temp/c
  if [[ "$bots" != "N" && "$bots" != "n" ]]; then
    cd ../../../../xpilot-ai-temp/c
    wget http://xpilot-ai.org/downloads/example%20bots/Helper.c
    wget http://xpilot-ai.org/downloads/example%20bots/Spinner.c
    wget http://xpilot-ai.org/downloads/example%20bots/TesterBot.c
    cd ../../xpilot-ng-ai/src/client/x11
  fi
fi
# Download and build Java library
echo;echo -n "Java: Y/n --> "
read choice
if [[ "$choice" != "N" && "$choice" != "n" ]]; then
  sudo apt-get -y install openjdk-7-jdk
  wget http://xpilot-ai.org/downloads/xpilot-ng-ai-0.9/javaAI.c
  wget http://xpilot-ai.org/downloads/xpilot-ng-ai-0.9/javaAI.java
  javac javaAI.java
  javah -jni javaAI
  gcc -w -fPIC -Wl,-wrap,Handle_end -DHAVE_CONFIG_H -I. -I../../..  -DCONF_DATADIR=\"../../../share/xpilot-ng/\" -I../../../src/common -I../../../src/client -I/usr/lib/jvm/java-7-openjdk-amd64/include/ -I/usr/lib/jvm/java-7-openjdk-amd64/include/linux -I/usr/lib/jvm/java-7-openjdk-i386/include/ -I/usr/lib/jvm/java-7-openjdk-i386/include/linux -g -O2 -MT javaAI.o -MD -MP -MF .deps/javaAI.Tpo -c -o javaAI.o javaAI.c
  gcc -o libjavaAI.so -fPIC -shared -Wl,-soname,libjavaAI.so,-wrap,Handle_end javaAI.o about.o bitmaps.o colors.o configure.o dbuff.o guimap.o guiobjects.o join.o paintdata.o painthud.o paintradar.o record.o talk.o welcome.o widget.o xdefault.o xevent.o xeventhandlers.o xinit.o xpaint.o xpilot.o ../../../src/client/libxpclient.a ../../../src/common/libxpcommon.a   -lSM -lICE   -lX11  -lexpat -lz -lm
  mkdir ../../../../xpilot-ai-temp/java
  mv javaAI.java ../../../../xpilot-ai-temp/java
  mv libjavaAI.so ../../../../xpilot-ai-temp/java
  if [[ "$bots" != "N" && "$bots" != "n" ]]; then
    cd ../../../../xpilot-ai-temp/java
    wget http://xpilot-ai.org/downloads/example%20bots/Helper.java
    wget http://xpilot-ai.org/downloads/example%20bots/Spinner.java
    wget http://xpilot-ai.org/downloads/example%20bots/TesterBot.java
    cd ../../xpilot-ng-ai/src/client/x11
  fi
fi
# Download and build Python library
echo;echo -n "Python: Y/n --> "
read choice
if [[ "$choice" != "N" && "$choice" != "n" ]]; then
  sudo apt-get -y install python3-dev
  wget http://xpilot-ai.org/downloads/xpilot-ng-ai-0.9/pyAI.c
  gcc -w -fPIC -Wl,-wrap,Handle_end -DHAVE_CONFIG_H -I. -I../../..  -DCONF_DATADIR=\"../../../share/xpilot-ng/\" -I../../../src/common -I../../../src/client -I/usr/include/python3.2 -g -O2 -MT pyAI.o -MD -MP -MF .deps/pyAI.Tpo -c -o pyAI.o pyAI.c
  gcc -o libpyAI.so -fPIC -shared -Wl,-soname,libpyAI.so,-wrap,Handle_end pyAI.o about.o bitmaps.o colors.o configure.o dbuff.o guimap.o guiobjects.o join.o paintdata.o painthud.o paintradar.o record.o talk.o welcome.o widget.o xdefault.o xevent.o xeventhandlers.o xinit.o xpaint.o xpilot.o ../../../src/client/libxpclient.a ../../../src/common/libxpcommon.a   -lSM -lICE   -lX11  -lexpat -lz -lm -lpython3.2mu
  mkdir ../../../../xpilot-ai-temp/python
  mv libpyAI.so ../../../../xpilot-ai-temp/python
  if [[ "$bots" != "N" && "$bots" != "n" ]]; then
    cd ../../../../xpilot-ai-temp/python
    wget http://xpilot-ai.org/downloads/example%20bots/Ambidextrous.py
    wget http://xpilot-ai.org/downloads/example%20bots/Helper.py
    wget http://xpilot-ai.org/downloads/example%20bots/Spinner.py
    wget http://xpilot-ai.org/downloads/example%20bots/TesterBot.py
    cd ../../xpilot-ng-ai/src/client/x11
  fi
fi
# Download and build Racket library
echo;echo -n "Racket (PLT-Scheme): Y/n --> "
read choice
if [[ "$choice" != "N" && "$choice" != "n" ]]; then
  sudo apt-get -y install racket
  wget http://xpilot-ai.org/downloads/xpilot-ng-ai-0.9/rktAI.c
  gcc -w -fPIC -Wl,-wrap,Handle_end -DHAVE_CONFIG_H -I. -I../../..  -DCONF_DATADIR=\"../../../share/xpilot-ng/\" -I../../../src/common -I../../../src/client -g -O2 -MT rktAI.o -MD -MP -MF .deps/rktAI.Tpo -c -o rktAI.o rktAI.c
  gcc -o librktAI.so -fPIC -shared -Wl,-soname,librktAI.so,-wrap,Handle_end rktAI.o about.o bitmaps.o colors.o configure.o dbuff.o guimap.o guiobjects.o join.o paintdata.o painthud.o paintradar.o record.o talk.o welcome.o widget.o xdefault.o xevent.o xeventhandlers.o xinit.o xpaint.o xpilot.o ../../../src/client/libxpclient.a ../../../src/common/libxpcommon.a   -lSM -lICE   -lX11  -lexpat -lz -lm
  mkdir ../../../../xpilot-ai-temp/racket
  mv librktAI.so ../../../../xpilot-ai-temp/racket
  if [[ "$bots" != "N" && "$bots" != "n" ]]; then
    cd ../../../../xpilot-ai-temp/racket
    wget http://xpilot-ai.org/downloads/xpilot-ng-ai-0.9/rktAI.rkt
    wget http://xpilot-ai.org/downloads/example%20bots/Helper.rkt
    wget http://xpilot-ai.org/downloads/example%20bots/Spinner.rkt
    wget http://xpilot-ai.org/downloads/example%20bots/TesterBot.rkt
    cd ../../xpilot-ng-ai/src/client/x11
  fi
fi
echo;echo "Cleaning up!"
cd ../../../..
rm -r xpilot-ng-ai
mv xpilot-ai-temp xpilot-ng-ai 
