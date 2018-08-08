#!/bin/bash
# script to run the map-generator in ../../xpilot-ng-temp/c/
# (it would be nice if we could do ALL the map generation from
#  scripts in this server directory)

cd ../../xpilot-ng-temp/c/
./buildMapgen.sh
export LD_LIBRARY_PATH=.
./runMapgen.sh 1 points.csv
