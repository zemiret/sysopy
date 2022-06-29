#!/bin/bash

mkdir search_dir
cd search_dir
mkdir small
mkdir medium
mkdir big

for i in {1..10}; do
	echo $i > "small/file$i.txt"
done

for i in {1..10}; do
	mkdir "small/dir$i"
	echo $i > "small/dir$i/file$i.txt"
done

for i in {1..10}; do
	mkdir "medium/dir$i"
	echo $i > "medium/dir$i/file$i.txt"

	for k in {1..10}; do
		mkdir "medium/dir$i/dir$k"
		echo $i > "medium/dir$i/dir$k/file$k.txt"
	done
done

for i in {1..1000}; do
	mkdir "big/dir$i"
	echo $i > "big/dir$i/file$i.txt"

	for k in {1..10}; do
		mkdir "big/dir$i/dir$k"
		echo $i > "big/dir$i/dir$k/file$k.txt"
	done
done

cd ..
