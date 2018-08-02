#! /bin/bash
. base_defense_react_config.sh

for idx in $(seq 1 $1)
do
    name="fixed"
    name="$name$idx"
    echo "./chaser 0 $idx $1 $name"
    konsole -e ./boids 0 $idx $1 -team 1 -color0 "#23221f" -join -name $name -shipShape fixed
    sleep 1
done

for idx in $(seq 1 $1)
do
    name="fixed"
    myid=$(($1+$idx))
    name="$name$myid"
    echo "./chaser 0 $myid $1 $name"
    konsole -e ./boids 0 $myid $1 -team 2 -color0 "#23221f" -color7 "#ff8000" -color5 "#750eee" -color6 "#ffffff" -join -name $name -shipShape fixed
    sleep 1
done
