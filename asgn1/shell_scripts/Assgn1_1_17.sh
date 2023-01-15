#!/bin/bash
gcd(){
    ! (( $1%$2 )) && echo $2 || gcd $2 $(( $1%$2 ))
}
cat $1 | rev | sort -n > $2
curr=$(sed -n "1"p $2)

for i in `seq 2 $(wc -l < $2)`
do
    next=$(sed -n "$i"p $2)
    num1=$curr
    num2=$next
    curr=`expr $num1 \* $num2 / $(gcd $curr $next)`
done
echo $curr