#! /bin/bash
. base_defense_react_config.sh
  # Quads are sent out first, followed by the fixedwings, then quads, etc
for idx in $(seq 1 $1)
do
  if [ `expr $idx % 2 | bc` -eq 0 ]; then
    name="quadAttack"
    name="$name$idx"
    echo "./defender 0 $idx $1 $objx $objy $attack_radius $fuelx $fuely $quadfuel $name"
    konsole -e ./defender 0 $idx $1 $objx $objy $attack_radius $fuelx $fuely $quadfuel $attacker_rubber -team 2 -color0 "#23221f" -join -name $name -shipShape quad 
  else 
    name="fixedAttack"
    name="$name$idx"
    echo "./defender 0 $idx $1 $objx $objy $attack_radius $fuelx $fuely $fixedfuel $name" 
    konsole -e ./defender 0 $idx $1 $objx $objy $attack_radius $fuelx $fuely $fixedfuel $attacker_rubber -team 2 -color0 "#23221f" -join -name $name -shipShape fixed 
  fi
    sleep 1  
done
