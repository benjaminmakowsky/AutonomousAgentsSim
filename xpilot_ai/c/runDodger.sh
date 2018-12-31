#! /bin/bash
. base_defense_react_config.sh

for idx in $(seq 1 $1)
do
    name="fixed"
    name="$name$idx"
    echo "./chaser 0 $idx $1 $name"
    konsole -e ./dodger 0 $idx $1 -team 2 -color0 "#23221f" -join -name $name -shipShape fixed
    sleep 1
done
