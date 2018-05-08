#! /bin/bash
. base_defense_react_config.sh
name="test"
echo "./mapDebugBot"
konsole -e ./mapDebugBot 0 -team 1 -color0 "#23221f" -join -name $name -shipShape "drone_tank" 
