#! /bin/bash

# given a map, this script generates a list of points corresponding to the map
# and places it into the ../../../xpilot-ai/c/ directory

# identifies the first and last lines of the ASCII map in the given mapfile
line1=$( grep -n '\EndOfMapdata' $1 | awk '//{i++}i==1' | grep -o -E "^[0-9]{1,3}" )
line2=$( grep -n '\EndOfMapdata' $1 | awk '//{i++}i==2' | grep -o -E "^[0-9]{1,3}" )
(( line1++ ))
(( line2-- ))

# pulls out the ASCII map
sed -n "$line1,$line2 p" $1 > parsemap.txt

# compiles and runs a program that will generate a marked-up version of the
# ASCII map and a resulting list of graph points
gcc -g getFuelPoints.c -o getFuelPoints
./getFuelPoints parsemap.txt

# removes the created parsemap file
#rm parsemap.txt

# moves the file with all the graph points to the ../../../xpilot-ai/c/ directory
mv fuelpoints.csv ../../../xpilot_ai/c/

