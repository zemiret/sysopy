#!/bin/bash

[[ -z $1 ]] && echo "Specify exercise number" && exit 1

todir=$(pwd)
if [[ -n $2 ]]; then
	todir=$2
fi

name="MleczkoAntoni"
exercise=$1
dirname=$name-$exercise
zipname="$dirname.tar.gz"

mkdir $dirname

cp -r $exercise $dirname
tar -czf $zipname $dirname
rm -r $dirname

if [[ $(pwd) != $todir ]]; then
	mv $zipname $todir
fi

