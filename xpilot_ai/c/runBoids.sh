#! /bin/bash
. base_defense_react_config.sh

# The loop below spawns $1 drones on each team, alternating between team1 and team2. 
for idx in $(seq 1 $((2*$1)))
do
    teamnum=$((($idx-1)%2+1))
    myname="fixed"$idx
    echo "./boids 0 $idx $1 $myname $teamnum"
    konsole -e "./boids 0 $idx $1 $teamnum -team $teamnum -join -name $myname -shipShape fixed" &
    sleep 1
done
