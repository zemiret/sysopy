## Compile
```
make all
```

## Synopsis
Tester:
```
./tester <monitoredfile> <pmin> <pmax> <bytes>
```
Program:
```
./out <monitorlist>
```

### Arguments 
Tester:
* monitoredfile - path to file which shall be written to
* pmin - minimum number of seconds to wait
* pax - maximum number of seconds to wait
* number of random bytes to append to monitored file

Program:
* monitorlist - file with monitored files and timeouts in format (as per upel.agh.edu.pl): `<filepath> <seconds>`.

### Commands
After the program is started you can supply commands to it. Description pasted from upel:

* LIST - program wypisuje listę procesów monitorujących pliki
* STOP PID - program zatrzymuje (nie kończy) monitorowanie procesu o numerze PID. Można to zrealizować poprzez wysłanie do procesu potomnego sygnału SIGUSR1. Proces macierzysty po odebraniu sygnału zatrzymuje pętlę poprzez zmianę flagi.
* STOP ALL - program zatrzymuje (nie kończy) monitorowanie wszystkich procesów potomnych
* START PID - program wznawia monitorowanie procesu o numerze PID (również poprzez wysłanie sygnału i zmianę flagi).
* START ALL - program wznawia działanie wszystkich procesów potomnych
* END - program kończy działanie wszystkich procesów i wyświetla końcowy raport.

## Examples
Examplar monitorlist:
```
/tmp/monitorme1 2
/tmp/monitorme2 4
```

Program calls:
```
./tester /tmp/monitorme1 1 5 100
./tester /tmp/monitorme2 2 7 13

./out ./monitorlist
```
