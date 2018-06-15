#! /bin/bash
#clientPath="/home/sanchezri1/Downloads/xpilot-le/xpilot-engine/src/client/x11/"
clientPath="../../src/client/x11/"

rm -f libcAI.so
cp cAI.h $clientPath
cd $clientPath
gcc -w -fPIC -Wl,-wrap,Handle_end -DHAVE_CONFIG_H -I. -I../../..  -DCONF_DATADIR=\"../../../share/xpilot-ng/\" -I../../../src/common -I../../../src/client -g -O2 -MT cAI.o -MD -MP -MF .deps/cAI.Tpo -c -o cAI.o cAI.c
gcc -o libcAI.so -fPIC -shared -Wl,-soname,libcAI.so,-wrap,Handle_end cAI.o about.o bitmaps.o colors.o configure.o dbuff.o guimap.o guiobjects.o join.o paintdata.o painthud.o paintradar.o record.o talk.o welcome.o widget.o xdefault.o xevent.o xeventhandlers.o xinit.o xpaint.o xpilot.o ../../../src/client/libxpclient.a ../../../src/common/libxpcommon.a   -lSM -lICE   -lX11  -lexpat -lz -lm
mv libcAI.so ../../../xpilot-ng-temp/c/
rm cAI.h
cd ../../../xpilot-ng-temp/c/
gcc mapgen.c libcAI.so -lpthread -lm -o mapgen
