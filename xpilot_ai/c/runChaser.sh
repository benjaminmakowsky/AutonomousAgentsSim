#! /bin/bash
. base_defense_react_config.sh

for idx in $(seq 1 $1)
do
    name="fixed"
    name="$name$idx"
    echo "./chaser 0 $idx $1 $name"
    #konsole -e ./chaser 0 $idx $1 -team 1 -color0 "#23221f" -join -name $name -shipShape fixed -maxFPS 5
    konsole -e ./chaser 0 $idx $1 $2 -color0 "#23221f" -join -name $name -shipShape fixed
    sleep 1
done
