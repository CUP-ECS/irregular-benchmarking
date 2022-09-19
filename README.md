# miniapp-benchmarking
Collection of scripts to perform analysis and benchmarking on parameters extracted from various mini-apps.

## Benchmark
### Benchmark Dependencies
To build/run the benchmark, you will need:
- C/C++ compiler
- MPI
- CMake

Optionally, if your system supports the following options, you can build for them:
- CUDA
- OpenMP
- OpenCL

### Building Benchmark
To build the benchmark, run the following script (or change it for your needs):
```bash
# create a build directory for your benchmark to be built with
mkdir build

# move into the newly created directory
cd build

# configure the Makefile
cmake ..

# build the benchmark
make -j 8
```

## Scripts
There are a few scripts available.
Their functionality, and how to use them, are listed below.

### Benchmark Runner
This script runs the benchmark after your parameters are extracted from a target application.

#### Dependencies
Before running the `benchmark-runner.py` script, you will need the following:
- Python 3
- All of the benchmark dependencies

#### Running the Benchmark
To run the benchmark, you will use the following command:

```bash
python3 benchmark-runner.py [PARAM_FILE_PATH]
```
You must specify a path to a file where your parameter data is stored.
The format that this data must be in will be described in a following section.

Unless specified via an argument, the runner will bootstrap the benchmark from scratch and place it in a `.benchmark_stage` directory.
The benchmark binary will then be copied into a `results` directory where it can be run and stored in isolation.
The name of the results directory can be changed via the `-r` flag.





