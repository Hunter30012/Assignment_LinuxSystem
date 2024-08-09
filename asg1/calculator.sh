#!/usr/bin/bash

if [ $# -ne 3 ]; then
    echo "The shell script execute with 3 argument"
    echo "Please run again!"
    exit 1
fi

num1=$1
num2=$3
operator=$2

if [ $operator = "+" ]; then
    sum=$((num1 + num2))
    echo "$sum"
elif [ $operator = "-" ]; then
    difference=$((num1 - num2))
    echo "$difference"
elif [ $operator = "x" ]; then
    product=$((num1 * num2))
    echo "$product"
elif [ $operator = "/" ]; then
    if [ $num2 -ne 0 ]; then
        quotient=$((num1 / num2))
        echo "$quotient"
    else
        echo "Error: Division by zero"
    fi
else
    echo "operator invalid"
fi
