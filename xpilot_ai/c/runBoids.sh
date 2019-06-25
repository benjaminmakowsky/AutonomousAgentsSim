#! /bin/bash
export LD_LIBRARY_PATH=.
. base_defense_react_config.sh

# The loop below spawns $1 drones on each team, alternating between team1 and team2. 
for idx in $(seq 1 $((2*$1)))
do
    teamnum=$((($idx-1)%2+1))
    myname="fixed"$idx
    echo "./boids 0 $idx $1 $myname $teamnum"
    ./boids 0 $idx $1 $teamnum -team $teamnum -join -name $myname -shipShape fixed &
    #./boids 0 $idx $1 $teamnum -team $teamnum -join -name $myname -shipShape quad &
    sleep 3
done
