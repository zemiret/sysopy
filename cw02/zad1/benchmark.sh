#!/bin/bash

make all > /dev/null

echo "Built binaries." 
echo "---" 

record_sizes=(1 4 512 1024 4096 8192)
record_counts=(10000 8000 6000 7000 5000 5000)

echo "Benchmark statistics:"  
echo ""  
for i in ${!record_sizes[*]}; do
	echo "Record size: ${record_sizes[i]}, record count: ${record_counts[i]}" 

	./out generate data ${record_counts[i]} ${record_sizes[i]} 
	cp data dataLib
	cp data dataSys

	echo "Sorting:" 
	echo -n "lib: " 
	time ./out sort dataLib ${record_counts[i]} ${record_sizes[i]} lib 2>&1 
	echo -n "sys: " 
	time ./out sort dataSys ${record_counts[i]} ${record_sizes[i]} sys  2>&1 

	echo "Copying:" 
	echo -n "lib: " 
	time ./out copy data dataLibCopy ${record_counts[i]} ${record_sizes[i]} lib 2>&1
	echo -n "sys: " 
	time ./out copy data dataSysCopy ${record_counts[i]} ${record_sizes[i]} lib  2>&1

	echo "---" 

	rm data dataSys dataLib dataLibCopy dataSysCopy
done
