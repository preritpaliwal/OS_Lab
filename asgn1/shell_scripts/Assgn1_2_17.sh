for i in `cat<$1|tr [A-Z] [a-z]`
do
    for j in `cat<$3|tr [A-Z] [a-z]`
    do
        [[ "$i" == *"$j"* ]]&&flag=1&&break||flag=0
    done
    ((${#i}>=5&&${#i}<=20))&&!((flag))&&[[ "$i" =~ ^[a-z]+[0-9][a-z0-9]*$ ]]&&echo "Yes">>$2|| echo "No">>$2
done


