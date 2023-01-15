#!/bin/bash
for i in `cat < $1 | tr [A-Z] [a-z]`
do
    len=`expr length $i`
    if ( ((len >= 5 && len <= 20)) && [[ "$i" =~ ^[a-z]+[0-9][a-z0-9]*$ ]]) 
    then 
        flag=0
        for j in `cat < $3 | tr [A-Z] [a-z]`
        do
            if [[ "$i" == *"$j"* ]]
            then
                echo "No" >> $2
                flag=1
                break
            fi
        done
        if (($flag == 0))
        then
            echo "Yes" >> $2
        fi

    else
        echo "No" >> $2
    fi
done