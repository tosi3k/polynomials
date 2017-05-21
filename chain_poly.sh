#!/bin/bash

if [[ $# -ne 2 ]]; then
    echo "Zła liczba argumentów"
    exit 1
elif [[ ! -x  $1 ]]; then
    echo "Podana nazwa programu nie wskazuje na plik wykonywalny"
    exit 1
elif [[ ! -d $2 ]]; then
    echo "Podana nazwa katalogu nie wskazuje na katalog"
    exit 1
fi

exe=$1
d=$2

filename=$(echo $(echo $(find $d -maxdepth 1 -type f -exec sed '/START/F;Q' {} \;) | sed 's/.*\///'))

out=""
i=0

while :
do
    
    f=$(cat "$d/$filename")
    
    lastline=${f##*$'\n'}
    
    if (( $i == 0 )); then
        f=${f#*$'\n'}
        i=1
    fi
    
    f=$(echo "$f" | sed '$d')
    
    if [[ $out != "" ]] && [[ $f != "" ]]; then
        out=$out$'\n'$f
    elif [[ $out == "" ]]; then
        out=$f
    fi
    
    if [[ $lastline == "STOP" ]]; then
        if [[ $out != "" ]]; then
            ./$exe <<< """$out"""
        fi
        exit 0
    else
        if [[ $out != "" ]]; then
            out=$(echo -n "$out" | ./$exe)
        fi
        filename=${lastline/FILE\ /}
    fi
done
