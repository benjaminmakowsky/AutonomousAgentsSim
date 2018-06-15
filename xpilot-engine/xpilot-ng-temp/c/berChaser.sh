#!/bin/bash

# wrapper to build the chaser.c file, export LD_LIBRARY_PATH as the current
# directory, and run the chaser executable with some number of drones.
./buildChaser.sh
export LD_LIBRARY_PATH=.
./runChaser.sh $1 map.csv
