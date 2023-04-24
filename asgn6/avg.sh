#!/bin/bash

sum=0.0
count=0
while read line
do
    sum=`echo $sum + $line | bc`
    count=`expr $count + 1`
    echo $sum $count
done < $1
echo "scale=2; $sum / $count" | bc