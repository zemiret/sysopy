## Compile
```
make all
```

## Synopsis
*master*
```
./master.out <fifopath>
```
*slave*
```
./slave.out <fifopath> <N>
```

*Arguments:*
* fifopath - the path to file for which fifo is created
* N - the count of communicates to write

## Example
```
./master kanapeczki

// and then
./slave kanapeczki 100
./slave kanapeczki 10
./slave kanapeczki 44
```
