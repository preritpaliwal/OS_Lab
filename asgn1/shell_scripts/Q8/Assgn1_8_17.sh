#!/bin/bash

# Helper Function for help option
Help(){
    
    printf "NAME \n"
    printf "\tCSV File Manipulator - A Expense Tracker for when you go out with your friends and choose to use a terminal based program for no apparent reason. (Splitwise FTW) \n"
    printf "\n"

    printf "SYNOPSIS \n"
    printf "\t./Assgn1_8_17.sh [OPTION] [RECORD] \n"
    printf "\t./Assgn1_8_17.sh [RECORD] \n"
    printf "\n"

    printf "DESCRIPTION \n"
    printf ""    

    printf "\t-c \n"
    printf "\t Accepts a category of expense and outputs the amount of money spent in that category. \n\n"
    printf "\t-n \n"
    printf "\t Accepts a name of a person and print the amount spent by that person. \n\n"
    printf "\t-s \n"
    printf "\t Accepts a column name of CSV Sort the csv by column name.\n"
    printf "\t Valid arguments are : {date, category, amount, name} \n\n"
    printf "\t-h \n"
    printf "\t Show  Help Prompt. \n\n"
}

# If you want the column headers in a new file, use this

# if [[ ! -e temp.csv ]]; then
#     echo "Date,Category,Amount,Name" > temp.csv
# fi

# Else, use this
touch main.csv

# There could be any number of arguments but the entry record would be the last 4 arguments
if [[ $# -gt 3 ]]; then
    check="${@:(-2):1}"
    firstchar=${check:0:1}
    if [[ "$firstchar" != "-" ]]; then
        record="${@: -4}"
        arr=($record)
        echo ${arr[0]},${arr[1]},${arr[2]},${arr[3]} >> main.csv
    fi
fi


sort -k1 -t, main.csv > temp.csv
mv temp.csv  main.csv

# Get the options and process them
# The colon after option initials represent that the option requires an additional argument.
while getopts "c:n:s:h" option; do
    case $option in 

    c) # Category
        category=$OPTARG
        sum=0
        # sed "/$category/!d" main.csv | cat
        while IFS=, read -r a b c d;
        do
            if [[ "$b" = "$category" ]]; then
                sum=`expr $sum + $c`
            fi
        done < main.csv 
            echo "$sum rupees spent on $category."
        ;;

    n) # Name
        name=$OPTARG
        sum=0
        while IFS=, read -r a b c d;
        do
            if [[ "$d" = "$name" ]]; then
                sum=`expr $sum + $c`
            fi
        done < main.csv
            echo "$sum rupees spent by $name."
        ;;

    s) # Sort by Column Name in $OPTARG
        col=$OPTARG

        # Sort the csv based on the date  in that entry
        if [[ "$col" = "date" ]]; then
            sort -k1 -t, main.csv > temp.csv
            mv temp.csv  main.csv

        # Sort the csv based on the name of category in that entry
        elif [[ "$col" = "category" ]]; then
            sort -k2 -t, main.csv > temp.csv
            mv temp.csv  main.csv
        
        # Sort the csv based on amount of money spent in that entry in descending order
        elif [[ "$col" = "amount" ]]; then
            sort -k3 -n -r -t, main.csv > temp.csv
            mv temp.csv  main.csv      

        # Sort the csv based on the name of person in that entry
        elif [[ "$col" = "name" ]]; then
            sort -k4 -t, main.csv > temp.csv
            mv temp.csv  main.csv 
        
        else
            echo "Invalid Column for Sorting! Terminating Script."
            exit

        fi

        ;;

    h) # Display Help
        Help
        ;;

    \?) # Invalid Option
        echo "Invalid Option! Terminating Script."
        exit
        ;;
    
    esac
done
shift "$(($OPTIND -1))"