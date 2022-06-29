## Compile
```
make all
```

## Synopsis
Catcher:
```
./catcher.out <type> 
```
Program:
```
./sender.out <catcherPid> <sigcount> <type>
```

### Arguments 
* `type` - one of KILL, SIGQUEUE, SIGRT - the type of messaging used to send signals
* `catcherPid` - catcher's pid. It's printed when catcher is started 
* `sigcount` - signals count to be send

## Examples
Examplar monitorlist:
```
./catcher.out KILL
./sender.out 25336 1000 KILL

./catcher.out SIGQUEUE 
./sender.out 25336 1000 SIGQUEUE 

./catcher.out SIGRT 
./sender.out 25336 1000 SIGRT 
```

