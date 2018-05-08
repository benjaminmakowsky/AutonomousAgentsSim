#! /bin/bash
. base_defense_react_config.sh
# tanks are sent out first, followed by the infantry, then tank, etc
name="tank"
echo "./defender 0 1 1 $objx $objy $react_radius $fuelx $fuely $tankfuel $name"
konsole -e ./defender 0 1 1 $objx $objy $react_radius $fuelx $fuely $tankfuel $react_rubber 1 $gridNum $gridrally_00x $gridrally_00y $gridrally_01x  $gridrally_01y $gridrally_02x $gridrally_02y $gridrally_10x $gridrally_10y $gridrally_20x $gridrally_20y $gridrally_11x $gridrally_11y $gridrally_12x $gridrally_12y $gridrally_21x $gridrally_21y $gridrally_22x $gridrally_22y -team 0 -color0 "#23221f" -join -name $name -shipShape drone_tank 
