#! /bin/bash
. base_defense_react_config.sh
  # Quads are sent out first, followed by the fixedwings, then quads, etc
for idx in $(seq 1 $1)
do
  if [ `expr $idx % 2 | bc` -eq 0 ]; then
    name="quad"
    name="$name$idx"
    echo "./defender 0 $idx $1 $objx $objy $def_radius $fuelx $fuely $quadfuel  $def_rubber 0 -team 1 -color0 '#23221f' -join -name $name -shipShape quad"
    konsole -e ./defender 0 $idx $1 $objx $objy $def_radius $fuelx $fuely $quadfuel $def_rubber 0 -team 1 -color0 "#23221f" -join -name $name -shipShape quad 
  else 
    name="fixed"
    name="$name$idx"
    echo "./defender 0 $idx $1 $objx $objy $def_radius $fuelx $fuely $fixedfuel $def_rubber 0 -team 1 -color0 '#23221f' -join -name $name -shipShape fixed"  
    konsole -e ./defender 0 $idx $1 $objx $objy $def_radius $fuelx $fuely $fixedfuel $def_rubber 0 -team 1 -color0 "#23221f" -join -name $name -shipShape fixed 
  fi
    sleep 1  
done
