## Compile
```
make all
```

## Sysopsis
*Trucker:*
```
./trucker.out truckerMassCapacity beltCapacity beltMassCapacity
```

Arguments:

* truckerMassCapacity - maximum total mass of boxes that trucker have loaded
* beltCapacity - number of boxes that can be on belt at once
* beltMassCapacity - maximum total mass of boxes that can be on belt

*Loader (executor)*
```
./loader_executor.out workersCount maxBoxMass [cyclesCount]
```

Arguments:

* workersCount - number of workers that will put packages
* maxBoxMass - self explanatory
* cyclesCount - optional. Number of box loadings that will be performed by each worker 

*(individual) Loader*
```
./loader.out boxMass [cyclesCount]
```

Arguments:

* boxMass - mass of the boxes that worker will load
* cyclesCount - optional. Number of box loadings that will be performed by each worker 

## Examples:
```
./trucker.out 300 10 100

// in separate console
./loader_executor.out 20 100 100
```
