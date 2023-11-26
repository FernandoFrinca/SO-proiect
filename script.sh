#!bin/bash

#if [ $# -eq 1 ]; then
    #echo "Numărul corect de argumente: 1"
if [ $# -ge 2 ]; then
    #echo "Numărul incorect de argumente: prea multe"
    exit 1
elif [ $# -eq 0 ]; then
    #echo "Numărul incorect de argumente: prea putine"
    exit 1
fi

caracter=$1
count=0
regex="^[A-Z][a-zA-Z0-9 ,]*[$caracter][a-zA-Z0-9 ,]*[.!?]{1}$"

while read -r line; do
    length=${#line}

    if [ $length -eq 0 ]; then
        break
    fi
     if echo "$line" | grep -E -q "$regex" && \
        echo "$line" | grep -E -q -v ",[ ]*si"; then
        count=$((count + 1))
    fi

done

echo "$count"
