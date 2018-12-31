#! /bin/bash
. base_defense_react_config.sh

# The loop below spawns $1 drones on each team, alternating between team1 and team2. 
for idx in $(seq 1 $((2*$1)))
do
    teamnum=$((($idx-1)%2+1))
    name="fixed"$idx
    echo "./boids 0 $idx $1 $name"
    konsole -e ./boids 0 $idx $1 $teamnum -team $teamnum -color0 "#23221f" -color7 "#ff8000" -color5 "#750eee" -color6 "#ffffff" -join -name $name -shipShape fixed 
    sleep 1
done
