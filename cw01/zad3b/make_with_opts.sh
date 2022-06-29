rm static dynamic shared
echo "O0" >> results3b.txt
make all OPT=-O0 >> results3b.txt

rm static dynamic shared
echo "O1" >> results3b.txt
make all OPT=-O1 >> results3b.txt

rm static dynamic shared
echo "O2" >> results3b.txt
make all OPT=-O2 >> results3b.txt

rm static dynamic shared
echo "O3" >> results3b.txt
make all OPT=-O3 >> results3b.txt

rm static dynamic shared
echo "Os" >> results3b.txt
make all OPT=-Os >> results3b.txt

rm static dynamic shared
echo "Ofast" >> results3b.txt
make all OPT=-Ofast >> results3b.txt

