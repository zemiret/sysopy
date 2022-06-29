## Compile
```
make all
```

## Synopsis
```
./main.out <threads> <type> <input> <filter> <output> 
```

### Arguments 
* threads - number of threads to run on
* type - either block or interleaved. Which type of work sharing to use
* input - path to input file in pgm format
* filter - path to filter file in pgm format
* output - path to output file

## Examples
```
./main.out 8 interleaved res/baboon.ascii.pgm res/filter48.pgm out.pgm

./main.out 8 block res/baboon.ascii.pgm res/filter48.pgm out.pgm
```

## Benchmarking
To see running times for given threads count, filter and type run.
```
./benchmark.sh
```
Results should be saved in Times.txt file.
