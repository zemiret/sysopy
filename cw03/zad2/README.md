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
./out <monitorlist> <timeout> <type>
```

### Arguments 
Tester:
* monitoredfile - path to file which shall be written to
* pmin - minimum number of seconds to wait
* pax - maximum number of seconds to wait
* number of random bytes to append to monitored file

Program:
* monitorlist - file with monitored files and timeouts in format (as per upel.agh.edu.pl): `<filepath> <seconds>`.
* timeout - time for which the program should monitor files
* type - archive or copy - chooses between operations in archive mode (file copied to memory) and copy mode (using exec function to call cp).

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

./out ./monitorlist 21 archive
./out ./monitorlist 21 copy
```
