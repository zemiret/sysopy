## Compile
```
make all
```

## Synopsis

### Server:
```
./server.out <sockname> <port> 
```

### Client
There are two possible ways to call client.
Eiter using LOCAL (local network) or UNIX sockets.

LOCAL:
```
./client.out <name> LOCAL <ip> <port>
```

UNIX:
```
./client.out <name> UNIX <sockname>
```


## Arguments 

### Server
* sockname - path to unix socket to be created
* port - port number for network socket

### Client 
* name - name under which client will be registered in server
* sockname - path to unix socket to be created
* ip - ip to which client will try to connect to
* port - port to which client will try to connect to


## Examples
```
./server.out gumisie 40000

# In different processess

./client.out abc UNIX gumisie
./client.out kanapka LOCAL 127.0.0.1 40000

# Then in server terminal - send job
Makefile
```
