#!/bin/bash
#default parameter is 1
TEAM=${1:-"1"}
echo "$TEAM"
./xpilot-ng-x11 -join -name observer -shipShape observer -team "$TEAM" -scaleFactor 1.8 
