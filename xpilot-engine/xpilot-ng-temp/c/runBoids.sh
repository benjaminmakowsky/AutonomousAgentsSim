#! /bin/bash
. base_defense_react_config.sh

for val in $(seq 1 $1)
do
    teamnum=1
    idx=$val
    name="fixed"
    name="$name$idx"
    echo "./boids 0 $idx $1 $name"
    konsole -e ./boids 0 $idx $1 $teamnum -team $teamnum -color0 "#23221f" -join -name $name -shipShape fixed
    sleep 1
done

for val in $(seq 1 $1)
do
    teamnum=2
    idx=$(($1+$val))
    name="fixed"
    name="$name$idx"
    echo "./boids 0 $idx $1 $name"
    konsole -e ./boids 0 $idx $1 $teamnum -team $teamnum -color0 "#23221f" -color7 "#ff8000" -color5 "#750eee" -color6 "#ffffff" -join -name $name -shipShape fixed
    sleep 1
done
