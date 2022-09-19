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

## Parameter Data Formatting
TODO: describe how the parameter data needs to be structured to be properly parsed by the scripts.

## Scripts
There are a few scripts available.
Their functionality, and how to use them, are listed below.

### Benchmark Runner
This script runs the benchmark after your parameters are extracted from a target application.
This application will bootstrap the benchmark for you, so building the benchmark manually is not required.

**NOTE:** Because of this, you should be sure to load **ALL** of the dependencies that you will need for the benchmark, otherwise this script might fail.

#### Dependencies
Before running the `benchmark-runner.py` script, you will need the following:
- Python 3
- All of the benchmark dependencies loaded into your shell before running the script

#### Running the Benchmark
To run the benchmark, you will use the following command:

```bash
$ python3 benchmark-runner.py --help
usage: benchmark-runner.py [-h] [-b [BPATH]] [-r [RPATH]] [param_path]

positional arguments:
  param_path  Specify path to file where parameter data is stored

options:
  -h, --help  show this help message and exit
  -b [BPATH]  Overrides where benchmark is built
  -r [RPATH]  Overrides where results are stored
```
You must specify a path to a file where your parameter data is stored.

Currently, the runner script is designed for machines running slurm.





