#!/bin/bash

threads="1 2 4 8"
filters="filter3.pgm filter18.pgm filter48.pgm"
types="block interleaved"
input='res/baboon.ascii.pgm'

make all > /dev/null

resfile='Times.txt'
out='benchmark_out.pgm'

rm $resfile

for f in $filters; do
	for typ in $types; do
		for th in $threads; do
			echo "$f $typ $th"
			out="$f_$typ_$th.pgm"

			echo "Filter: $f" >> $resfile
			echo "Type: $typ" >> $resfile
			echo "Threads: $th" >> $resfile
			./main.out $th $typ $input res/$f $out >> $resfile	
			echo "" >> $resfile
		done
	done
done

rm *.pgm

