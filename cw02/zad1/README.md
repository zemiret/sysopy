## Compile
```
make all
```

## Usage (as from upel.agh.edu.pl):

#### Generate
`./out generate dane 100 512` - powinno losowo generować 100 rekordów o długości 512 bajtów
   do pliku dane.

#### Sort
`./out sort dane 100 512 sys` - powinien sortować rekordy w pliku dane przy użyciu funkcji systemowych, 
    zakładając że zawiera on 100 rekordów wielkości 512 bajtów<br>
`./out sort dane 100 512 lib` -  powinien sortować rekordy w pliku dane przy użyciu bilioteki standardowej c, 
    zakładając że zawiera on 100 rekordów wielkości 512 bajtów

#### Copy
`./out copy plik1 plik2 100 512 lib` - powinno skopiować 100 rekordów pliku 1 do pliku 2 za pomocą funkcji 
    bibliotecznych z wykorzystaniem bufora 512 bajtów<br>
`./out copy plik1 plik2 100 512 sys` - powinno skopiować 100 rekordów pliku 1 do pliku 2 za pomocą funkcji 
    systemowych z wykorzystaniem bufora 512 bajtów

## Benchmarking
To benchmark the program, run
```
./benchmark.sh 
```

