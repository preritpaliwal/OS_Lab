#!/bin/bash

# find ./nmt-master/ -type f -name "*.py" > python_files.txt
find $1 -name "*.py" > python_files.txt

while IFS= read -r line; do
    
    # Print Filename and Filepath
    filename="$(basename $line)"
    echo "FILE NAME : $filename"
    echo "FILE PATH : $line"

    # Get octothorpe comments
    grep -no "#.*" $line
    # Works perfectly, also handles the case when there's some code followed by a single line comment

    # Get multiline comments using either """ or '''


    printf "\n\n"
done < python_files.txt




# Ideas Worth Trying

# grep -wno '#' ./nmt-master/nmt/train.py | cut -d: -f1 => Also has comments of type print("#some shit , text to be printed")

# awk '/"""/ {print NR; }' ./nmt-master/nmt/train.py  => Only prints line number as of now
# grep -n "\"\"\"" ./nmt-master/nmt/train.py => Gives a single output for lines containing two """ and also need to handle ending """
# grep -no "\"\"\"" ./nmt-master/nmt/train.py => Gives line number of both starting and ending line
# while IFS= read -r line; do awk '{if(NR==$line) print $0}' ./nmt-master/nmt/train.py; done < lines.txt => No Idea why this doesnt work

# Failed Ideas

    # grep -n "#" $line
    # This however prints the entire line which is bad for the case when you have some code followed by a comment