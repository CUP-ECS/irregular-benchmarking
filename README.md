# irregular-benchmarking
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

## Parameter Data File Formatting
Creating a parameter file is extremely simple, however it is important that formatting is correct or it will be unable to be parsed by the scripts.
The currently supported parameters are as follows:
- nowned
- nremote
- num_comm_partners
- blocksize
- stride

In your application, have the application print the desired parameters in the format:
```bash
PARAM: PARAMETER_NAME - PARAMETER_VALUE
```
For example, if you wanted to print that the `stride` parameter has a value of `12`, you would print the following to your log file:
```bash
PARAM: stride - 12
```
You can print the value as often as you deem appropriate.
The parser takes all instances of a parameter in a file and averages it to get the value used for the benchmark.
Failing to report a parameter results in the default value of 0.
Obviously, this could cause failures depending on the parameter so be sure to check that your values are being parsed correctly.
Write all of this data to a **single** text file and save it in a location the scripts can read from.
For an example parameter file, see `examples/example_clamr_parameter.txt`.
You can run any of the scripts in the repository on this parameter file to see how it behaves.

## Scripts
There are a few scripts available.
Their functionality, and how to use them, are listed below.

### benchmark-runner.py
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

### analysis.py
This scripts performs statistical analysis on the parameters extracted from applications.
It does not require that you have the benchmark dependencies, only that you have the required python packages.

#### Dependencies
Before running the `benchmark-runner.py` script, you will need the following:
- Python 3
- matplotlib
- scipy

#### Running the Benchmark
To run the benchmark, you will use the following command:

```bash
$ python3 analysis.py --help
usage: analysis.py [-h] [-r [RPATH]] [-c] [param_path]

positional arguments:
  param_path   Specify path to file where parameter data is stored

options:
  -h, --help   show this help message and exit
  -r [RPATH]   Overrides where results are stored
  -c, --clean  Removes previously generated files
```
You must specify a path to a file where your parameter data is stored.
