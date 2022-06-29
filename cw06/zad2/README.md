## Compile
```
make all
```

## Synopsis
Server
```
./server.out
```

Client(s)
```
./client.out [cmdfile]
```

*Arguments:*
* cmdfile - path to file containing lines of commands

If client is run with argument, it reads commands from file
and executes them using 1 second sleep in between them
(for readability).
If client is run without arguments, it reads commands from
the command line

## Example
Examplar cmdfile is provided in `res` folder.
Running with examplar cmdfile:
```
// Run each executable in separate process

./server.out

./client.out res/messages1
./client.out res/messages1
./client.out res/messages1
```

Exceprt from `res/messages1` file:
```
ECHO Start of sequence

LIST
2ALL Kanapeczki dla wszystkich

2ONE 0 Kanapeczki dla 0
2ONE 1 Kanapeczki dla 1
2ONE 2 Kanapeczki dla 2

FRIENDS 0 1 2
```
