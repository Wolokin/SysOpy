#!/bin/bash

# $1 - number count

for((i=0; i<$1; i++)); do
    echo $RANDOM >> dane.txt
done
