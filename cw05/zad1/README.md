## Compile
```
make all
```

## Synopsis
```
./main.out <cmdfile>
```

*Arguments:*
* cmdfile - path to file containing lines of commands

## Example
```
./main.out commands
```
Example command file:
```
ps aux | grep amleczko | grep 1 | grep 3 | sort    |	 wc -l 
ps aux | head -100  | grep 3 | sort | grep k   |	 wc -l


ps aux | grep amleczko | grep 1 | grep 3 | sort    |	 wc -l 
```
