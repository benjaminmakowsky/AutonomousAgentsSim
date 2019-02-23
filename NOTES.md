# Some quick notes:
# Where The Executables Are

The installation location for the main executables is `./xpilot_bin`,
the original build location for the executables are:

```
  ./xpilot_engine/src/client/x11
  ./xpilot_engine/src/server
  ./xpilot_engine/mapedit
```

## Important Directories

```
./xpilot_ai/c/                       <-- this is where the AI bots are written
./xpilot_engine/src/client/x11/cAI.c     <-- this is where the AI-lib implementation is
./xpilot_engine/src/client/x11   <-- this is where player/bot input is taken in, and game is drawn
./xpilot_engine/src/server  <-- this is where most of the XPilot code lives
```

## Building And Running

```
## How to build Xpilot_engine:
$ cd ./xpilot_engine
$ ./build.sh

## How to run the server( do this before running a client )
$ cd ./xpilot_bin/server

$ ./base_defense_server
This is a script that executes the current rendition of the map with a bunch of 
configuration options that we've been playing around with.

## How to run the client ( so that you control the ship )
## First have the server running.
$ cd ./xpilot_bin/client
$ ./xpilot_-ng-x11  < username > < shipShape name >
username can be anything you want as long as there isn't already a unit connected with said name
shipShape should be one of the following: quad, fixed, drone_tank or infantry

## How to run an AI bot
First have the server running.
$ cd ./xpilot-ai/c

```

Compile the bot you want to run, example: `gcc Spinner.c libcAI.so -o Spinner`

I have some scripts in place to recompile the AI library and bots ( `buildDefender.sh`, `buildAI.sh`, `buildMpaDebugbot.sh` ). Take a look at them.

## Important Files

`./xpilot_engine/src/server/player.h`

This is where the `player_t` struct is defined. Which we have used to give the different 
ships specific stats. You can see how we give these ships specific stats in 
`player.c` ( see `int Init_player` ) 

`./xpilot_engine/src/server/update.c`
This is where all the magic happens. Specifically, in the `static void Update_players(void)` 
function. That's the function where the player states are updated, as far as movement, 
spawning, calculating damage, etc.

*IMPORTANT NOTE*: AI bots spawned by the AI library are considered to be HUMAN PLAYERS, not robots.
So you can safely ignore any functions that are specific to robots.

`./xpilot_engine/src/server/walls.c`
This is where the very important `void Move_player(player_t *pl)` & `void Turn_player(player_t *pl, bool push)` are.
You can see the formula used to determine the changes in position here.
Some important ones right of the bat:
(see in `../xpilot_engine/src/server/update.c` ): 

```
pl->acc.x = pl->power * tcos(pl->dir) / inert;      ( inert is pl->mass )
pl->acc.y = pl->power * tsin(pl->dir) / inert;
```

`/usr/local/share/xpilot-ng/shipshapes.txt`
This is where the dimensions, positions of thrusters and weapons of the different ships are defined.

## Important AI Config Script

A run-down of the files in `/xpilot_ai/c`

1. `base_defense_react_config.sh`

Various configuration settings that are used by defender bots. 

| objx | the x coordinate of the objective they will defend. |
| objy | the y coordinate of ..
| def_radius | The radius of the circular formation that the defender bots will make |
| def_rubber | The max distance that a defender will move away from the objective point |
| react_rubber | The max distance that a reactionary_defender will move away from the objective point |
| attacker_rubber | The max distance than r will move away from the objective point |
| gridNum | Used by the WIP ground-movement to determine the number of grids to generate. |
| gridRally... | Rally-points that will mark the valid grid points for the WIP ground movement. |
| fuelx & fuely | coordinates of the refueling station. |
| ...fuel | fuel-value at which a bot will go to refueling station to refuel |
| attack_radius | distance-to-enemy at which a bot will engage and attack. |

## Shipshapes

`shipshapes.txt` is a file that the Xpilot server parses to determine what custom 
ships are available.

I've appended four new models to the base `shipshapes.txt` ( quad, fixed, infantry, drone_tank ).
TBH I've forgotten which directory the Xpilot server looks at for `shipshapes.txt`, so here
is the `$ locate shipshapes.txt` of my system:

```
/home/$USER/Desktop/shipshapes.txt
/home/$USER/Downloads/xpilot-ng-4.7 (copy).3/lib/shipshapes.txt
/home/$USER/Downloads/xpilot-ng-4.7.3/lib/shipshapes.txt
/home/$USER/Downloads/xpilot-ng-4.7.3/share/xpilot-ng/shipshapes.txt
/usr/local/share/xpilot-ng/shipshapes.txt
```

I'd suggest the you figure out which shipshapes.txt contains quad,fixed,infantry & drone_tank, and 
copy that `shipshapes.txt` to all the other directories listed above.

## How Does The AI Code Get Injected

Some quick notes on how AI bots work in Xpilot:
The Xpilot server runs at 24 FPS currently, on each frame, the server will ask
each bot ( the vanilla code calls the ai bots "Robots" ) to "play", AKA execute 
some actions on this frame.

See `server/update.c`, specfically `void Update_objects(void)`.
See `server/robot.c`, specifically `static void Robot_play( player_t* pl )`

The Robot_play function calls the given bot's play function, and this is where the AI code from
`./xpilot_ai/c` gets injected.

## Defender

Here's a high-level look at the state machine for defender. There are 6 states, in order of priority:

1. `STATE_INIT`, if not initialized, always starts here.
2. `STATE_REFUELING`, if low on fuel, always go to refuel no matter what.
3. `STATE_ENGAGING`, if enemy in attack range, engage. 
4. `STATE_IN_POSITION`, if in position and no nearby enemies, do nothing. 
5. `STATE_HEADING_TO_POSITION`, if not in position then head to the rally position.
6. `STATE_DEAD`, if dead do nothing.

Here's a run-down / documentation of how `defender.c` works. The code is fairly self-explanatory.

1. main
Parse the command-line arguments. If you ran the run.. scripts, these should get populated on their own.
    1.a gridMovement. 
    If enabled, the unit will spawn with the WIP ground-movement restrictions.
  
    1.b isLeader
    If enabled, the unit will spawn with the WIP leader abilities. The leader is able to order other 
    units to cover his rally-point while he is away.
  
    1.c rallyx & rallyy calculation. 
    Each unit will be assigned a different index value based on the order in which they are spawned.
    These index ( idx ) is used to determine their rally position in the circular formation protecting
    the objective.
    The rallypoint ( rallyx, rallyy ) are calculated using the formula:
    xCircumferenceCoordinate = xCenter + radius * cos theta
    yCircumferenceCoordinate = yCenter + radius * sin theta
  
    1.d start
    Once this is executed, the robot is marked as *ready* and its AI_loop function will be executed on every 
    frame by the server.

2. AI_loop
    2.a thrust
    When called with a parameter of 1, the unit will move in the direction it is looking at.
    When called with a parameter of 0, it will turn off the thrusters.
  
    2.b talk_frames
    Is used by the leader unit. Once the leader engages an enemy unit, it sends his current coordinates 
    on the next frame, followed by a message saying he is moving towards the enemy in the next one.
  
    2.c talk
    Function that writes the given string to the chat-box and sends it.
  
    2.d refuel
    When called with a parameter of 1, the unit will extract fuel from a nearby fuel station ( if there is any ).
    When called with a parameter of 0, the unit will stop extracting fuel.
  
    2.e pl_fuel
    An *approximate* estimate of the amount of fuel in the current unit.
  
    2.f screenEnemyXId, screenEnemyYId
    Function that return the coordinates of an enemy of screen with the given id.
    "Screen" meaning that they appear on the default Xpilot screen, as in you can see them
    without enlarging the borders of the xpilot window.

3. Initialize
  Spawns a thread that executes lose_fuel, calculates the rally-point for the unit.

4. Lose_fuel 
  ~~This function is executed on a separate thread by using pthread_create on the first frame of execution.
  We have a mutex here to avoid race-conditions. It would be nice to be able to get the ship value
  through an AI function call.~~ Currently, we don't do this, but we may add 
  this back in at a later date.

5. Refueling
  Moves towards the refueling station if not in range. If in range, refuels the ship.

  5.a angleToTurn
  atan2 is used to find the angle between the unit's current position, and the object it wants to head 
  towards [https://math.stackexchange.com/questions/1201337/finding-the-angle-between-two-points](https://math.stackexchange.com/questions/1201337/finding-the-angle-between-two-points)
  We then used the AI function turnToDeg to have the unit turn towards that location. 
  Note: I believe that movement at the moment is kind-of jerky, and calculating the angleToTurn on more 
  frames may result in smoother movement.

6. HeadToPosition
  Heads to the rallypoint.

7. Engaging.
  Moves towards the closest enemy unit and shoots.
  If gridMovement is enabled, it will move to the gridPoint that is closest to said enemy.

- Rene Sanchez ( 12/02/2018 )
- Daniel (Dec. 2018)
