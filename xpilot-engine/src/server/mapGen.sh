#!/bin/bash

cd ../../xpilot-ng-temp/c/
./buildMapgen.sh
export LD_LIBRARY_PATH=.
./runMapgen.sh 1 points.csv
