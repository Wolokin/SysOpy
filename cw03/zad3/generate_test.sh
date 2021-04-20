#!/bin/bash 

for (( c=1; c<=5; c++ ))
do
   mkdir test
   cd test
   echo "bcd" > bad.txt
   echo "bcdabcd" > good.txt
   echo "p" > wrong.extension
   mkdir emptydir
   done
