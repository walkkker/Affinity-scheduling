# Coursework for Threaded Programming Part 3: Affinity scheduling

## Source file

There is only one source file "B175662.c" which is named by my exam number.


## Build the code on Cirrus

The program is builded via make. To build the program on Cirrus, we should get Intel compilers on Cirrus first:

```console
$ module load intel-compilers-19
```

then run:

```console
$ make
```

If you want to delete all generated files, run:

```console
$ make clean
```


## Usage

To run the program on the front-end of Cirrus:

```console
$ ./B175662
```

## Change number of threads

If you run on the login node of Cirrus, you can change the number of threads using the following command:

```console
$ export OMP_NUM_THREADS= [NUMBER]
```

For example,

```console
$ export OMP_NUM_THREADS= 24
```

## Run on backend compute nodes of Cirrus

If you want to run the program on the backend compute nodes of Cirrus, please use the batch system of Cirrus.