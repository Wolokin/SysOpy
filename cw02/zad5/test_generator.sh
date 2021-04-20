#!/bin/bash

# $2 - line char count
# $1 - overall char count

base64 -w $2 /dev/urandom | head -c $1 > tests/t1.txt
