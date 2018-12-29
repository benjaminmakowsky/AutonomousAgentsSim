# Xpilot-engine Install instructions:

These versions of XPilot use the (slightly) more modern GNU Autoconf system to 
generate the Makefiles, run the automated build shell script in the root of the
source tree (`xpilot-engine/build.sh`). This script runs configure which
automatically detects your system type and attempt to determine what libraries,
includes, and dependancies are available (and will stop if a critical component
is missing). After that's done, it will run 'make'; this usually takes a few 
minutes.  If there are errors, consult the `INSTALL` or `INSTALL.TXT` file.
  
After make is finished, build.sh runs `make install` to install the programs,
maps, and textures. Note that this requires root privileges if installing to
`/usr/local`, but we aren't doing that so you should be good.

The final step of the script is to install the contents of xpilot_bitmaps,
which install `ShipShapes.txt` ( the valid unit types ) & the unit bitmaps ( sprites ).

-Rene Sanchez (11/29/2018)
