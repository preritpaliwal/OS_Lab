#!/bin/bash

names=$(sort -n ${1}/*)
mkdir -p $2

for i in {a..z}
do
    touch $2/$i.txt

    for n in $names
    do
        if [[ ${n:0:1} == $i ]]; then
            echo $n >> $2/$i.txt
        fi
    done
done
