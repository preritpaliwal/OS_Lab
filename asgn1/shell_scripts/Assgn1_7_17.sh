names=$(sort -n ${1}/*)
mkdir -p $2

touch $2/{a..z}.txt

for n in $names
do  
    echo $n >> $2/${n:0:1}.txt
done