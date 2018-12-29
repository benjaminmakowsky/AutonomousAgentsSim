# Overview

To run servers, clients, and AI bots in XPilot, you will use primarily the
following directories:

```
  ./xpilot_ai/c/
  ./xpilot_engine/src/server/
  ./xpilot_engine/src/client/x11/
  ./xpilot_engine/src/mapedit/
```

Below are a brief description of what each of these directories contains/
is useful for. Also, notice that the directories mentioned above could
change, if we ever get the chance to clean up XPilot's messy directory
structure.

## `./xpilot_engine/src/server/`

The server/ directory contains all the source code for an XPilot server,
including code for players, walls, collisions, items, etc.

## `./xpilot_engine/pvp_base_defense_server`

Starts an Xpilot server. You can change the map the server 
file and changing the filename listed for the -map option.

## `./xpilot_engine/src/client/x11/`

This directory contains a file called cAI.c, which comprises a
library of functions used by the AI bots. If you want to make your own AI
bots, you can utilize the functionality found here for operations like 
turning, thrusting, getting the ID's of other ships on the map, and so
on. From this directory, you can also run your own client that you can
control (provided a server is already running in a different terminal) by 
running `./xpilot-ng-x11 -join -name <username> -shipshape <shipshape>`

## `./xpilot_engine/base_defense_client <username> <shipshape>`

where username can be any character string and shipshape refers to some
valid ship shape, like fixed, quad, drone-tank, or infantry.

## `./xpilot_ai/c/`

The code for all the AI bots (including chaser and boids) is found in
this directory, along with scripts and instructions for running those AI
bots.

## `./xpilot_engine/src/mapedit/`

This directory contains a map editor, useful for creating maps that can
be moved into the server/ directory and used for XPilot games. Testing
your AI bots on a variety of maps can be helpful in ensuring that the AI
you create is not tailored to any particular map.

Daniel (Dec. 2018)
Matthew Coffman - August 2018
