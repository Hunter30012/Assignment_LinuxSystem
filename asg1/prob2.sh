#!/usr/bin/bash

read num_elements
read -a input

if [ ${#input[*]} -ne $num_elements ]; then 
    echo "Number of element in array is not equal the number you entered"
    exit 1
fi

array=()
for ((i=0; i<100; i++)); do
    array[i]=0
done

for((j = 0; j < num_elements; j++)); do
    array[${input[j]}]=$((array[${input[j]}] + 1))
done

for ((i=0; i<num_elements; i++)); do
    if [ ${array[${input[i]}]} -eq 1 ]; then
        echo "${input[i]}"
    fi
done

