gcd(){
    !(($1%$2))&&echo $2||gcd $2 $(($1%$2))
}
cat $1|rev|sort -n>$2
curr=$(head -1 $2)
while read i
do
    curr=$(($curr*$i/$(gcd $curr $i) ))
done <$2
echo $curr
