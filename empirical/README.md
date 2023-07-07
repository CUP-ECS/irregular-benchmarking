# Empirical Distribution

This folder simply contains the empirical distribution generator extracted from our benchmark.
This is useful for checking the overall efficacy of our benchmark's empirical generator.
Often times, it is not necessary to run a full MPI program, as the random number generation occurs the same inside our outside the benchmark.
It is not impacted by communication or any other aspect of the benchmark except for the input file (`BENCHMARK_CONFIG`).

### Build
Run the following:

```
gcc empirical.c -o empirical
```

### Run
To generate a sample of parameter data, run the following against a `BENCHMARK_CONFIG` file of your choice.

```
./empirical -f `BENCHMARK_CONFIG` > output.txt
```