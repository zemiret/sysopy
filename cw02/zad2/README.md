## Compile
```
make all
```

## Synopsis
For 1st version (opendir, readdir, stat family):
```
./out <dirpath> <comparator> <date>
```
For 2nd version (nftw):

```
./out2 <dirpath> <comparator> <date>
```

### Arguments 
* dirpath - path to the directory in which search occurs
* comparator - '<' or '=' or '>' - character used to compare dates.\
E.g. If '<' is used, it will find files that have modification date smaller than\
the 3rd argument
* date - date in seconds since 1 january 1970

## Examples
```
./out /etc '<' 2652738682 
./out /etc '=' 2652738682 
./out /etc '>' 1000

./out2 /etc '<' 2652738682 
./out2 /etc '=' 2652738682 
./out2 /etc '>' 1000
```
