#!/bin/bash
declare -i limit=1000000

numbers=($(seq 2 $limit))

for ((i=4; i<=$limit; i+=2))
do
    numbers[i-2]=0
done

for ((i=3; i*i<=$limit; i+=2))
do
    if [ ${numbers[i-2]} -ne 0 ]
    then
        for ((j=i*i; j<=$limit; j+=i))
        do
            numbers[j-2]=0
        done
    fi
done

while read -r line;
do
  for val in "${numbers[@]}"
  do
    if (( val > 0 && val < line )); then
      echo -n "${val} " >> "output.txt"
    fi
  done
  echo "" >> "output.txt"
done < "input.txt"
