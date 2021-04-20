#!/bin/bash

# $1 - filename
# $2 - pair count
# $3 - line char count
# $4 - overall char count

echo -n "$2 merge_files" > tests/test$1.txt
for (( c=1; c<=$2; c++ ))
do
	base64 -w $3 /dev/urandom | head -c $4 > tests/test$1a$c.txt
	base64 -w $3 /dev/urandom | head -c $4 > tests/test$1b$c.txt
    echo -n " tests/test$1a$c.txt:" >> tests/test$1.txt
    echo -n "tests/test$1b$c.txt" >> tests/test$1.txt
done
echo -n " remove_table" >> tests/test$1.txt
