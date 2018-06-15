#! /bin/bash

# given a map, this script generates a list of points corresponding to the map
# and places it into the ../../xpilot-ng-temp/c/ directory

line1=$( grep -n '\EndOfMapdata' $1 | awk '//{i++}i==1' | grep -o -E "^[0-9]{1,3}" )
line2=$( grep -n '\EndOfMapdata' $1 | awk '//{i++}i==2' | grep -o -E "^[0-9]{1,3}" )
(( line1++ ))
(( line2-- ))

sed -n "$line1,$line2 p" $1 > parsemap.txt
gcc -g getMapPoints.c -o getMapPoints
./getMapPoints parsemap.txt
#rm parsemap.txt
mv points.csv ../../xpilot-ng-temp/c/

#gnome-terminal -x ./pvp_base_defense_server
#gnome-terminal -x ./mapGen.sh
#pgrep -P $$
