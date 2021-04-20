#!/bin/bash

if [[ $1 == '' ]]; then
    READ_SIZE=2
else
    READ_SIZE=$1
fi
if [[ $2 == '' ]]; then
    PROD=5
else
    PROD=$2
fi
if [[ $3 == '' ]]; then
    CON=1
else
    CON=$3
fi

DIR=tests
mkdir -p $DIR
mkfifo $DIR/pipe
> $DIR/out.txt
rm -f $DIR/if.txt
for ((i=1; i<=PROD; i++)); do
    rm -f $DIR/in$i.txt
    for ((j=1; j<=$(( 5*READ_SIZE )); j++)); do
        echo -n $i >> $DIR/in$i.txt
    done
    echo $DIR/in$i.txt >> $DIR/if.txt
done
for ((i=1; i<=PROD; i++)); do
    ./producer.out $DIR/pipe $i $DIR/in$i.txt $READ_SIZE &
    pids[${i}]=$!
done
for ((i=1; i<=CON; i++)); do
    ./consumer.out $DIR/pipe $DIR/out.txt $READ_SIZE &
    pids[${i}]=$!
done
for pid in ${pids[*]}; do
    wait $pid
done
echo $DIR/out.txt > $DIR/of.txt
rm $DIR/pipe

./verifier.out $DIR/if.txt $DIR/of.txt
