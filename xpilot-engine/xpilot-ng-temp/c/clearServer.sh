#!/bin/bash

./buildDefender.sh
export LD_LIBRARY_PATH=.
./runReactionary.sh 1
