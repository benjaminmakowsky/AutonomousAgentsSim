#! /bin/bash
. base_defense_react_config.sh
  # tanks are sent out first, followed by the infantry, then tank, etc
for idx in $(seq 1 $1)
do
  if [ `expr $idx % 2 | bc` -eq 0 ]; then
    name="tankAttack"
    name="$name$idx"
    echo "./defender 0 $idx $1 $objx $objy $attack_radius $fuelx $fuely $tankfuel $name"
    konsole -e ./defender 0 $idx $1 $objx $objy $attack_radius $fuelx $fuely $tankfuel -team 1 -color0 "#23221f" -join -name $name -shipShape drone_tank 
  else 
    name="infantryAttack"
    name="$name$idx"
    echo "./defender 0 $idx $1 $objx $objy $attack_radius $fuelx $fuely $infantryfuel $name" 
    konsole -e ./defender 0 $idx $1 $objx $objy $attack_radius $fuelx $fuely $infantryfuel -team 1 -color0 "#23221f" -join -name $name -shipShape infantry
  fi
    sleep 1  
done
