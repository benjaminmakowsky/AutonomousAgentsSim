#! /bin/bash
. base_defense_react_config.sh
  # tanks are sent out first, followed by the infantry, then tank, etc
for idx in $(seq 1 $1)
do
  if [ `expr $idx % 2 | bc` -eq 0 ]; then
    name="quad"
    #name="tank"
    name="$name$idx"
    echo "./defender 0 $idx $1 $objx $objy $react_radius $fuelx $fuely $tankfuel $name"
    konsole -e ./defender 0 $idx $1 $objx $objy $react_radius $fuelx $fuely $tankfuel $react_rubber 0 -team 1 -color0 "#23221f" -join -name $name -shipShape quad 
  else 
    name="fixed"
    #name="infantry"
    name="$name$idx"
    echo "./defender 0 $idx $1 $objx $objy $react_radius $fuelx $fuely $infantryfuel $name" 
    konsole -e ./defender 0 $idx $1 $objx $objy $react_radius $fuelx $fuely $infantryfuel $react_rubber 0 -team 1 -color0 "#23221f" -join -name $name -shipShape fixed
  fi
    sleep 1  
done
