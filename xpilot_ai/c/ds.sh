#! /bin/bash
export LD_LIBRARY_PATH=.
. base_defense_react_config.sh

teamnum=1

#./boids 0 1 11 $teamnum -team $teamnum -join -name a1 -shipShape quad -headlessMode true &
#sleep 1

#teamnum=2
#./boids 0 2 11 $teamnum -team $teamnum -join -name a2 -shipShape quad -headlessMode true &
#sleep 1
#./boids 0 3 11 $teamnum -team $teamnum -join -name a3 -shipShape quad -headlessMode true &
#sleep 1
#./boids 0 4 11 $teamnum -team $teamnum -join -name a4 -shipShape quad -headlessMode true &
#sleep 1
#./boids 0 5 11 $teamnum -team $teamnum -join -name a5 -shipShape quad -headlessMode true &
#sleep 1
#/boids 0 6 11 $teamnum -team $teamnum -join -name a6 -shipShape quad -headlessMode true &
#sleep 1
#./boids 0 7 11 $teamnum -team $teamnum -join -name a7 -shipShape quad -headlessMode true &
#sleep 1
#./boids 0 8 11 $teamnum -team $teamnum -join -name a8 -shipShape quad -headlessMode true &
#sleep 1
#./boids 0 9 11 $teamnum -team $teamnum -join -name a9 -shipShape quad -headlessMode true &
#sleep 1
#konsole -e "./boids 0 10 11 $teamnum -team $teamnum -join -name myname -shipShape fixed" &
#gdb --args boids 0 10 11 $teamnum -team $teamnum -join -name myname -shipShape fixed 
./boids 0 10 11 $teamnum -team $teamnum -join -name myname -shipShape fixed &
#sleep 1
