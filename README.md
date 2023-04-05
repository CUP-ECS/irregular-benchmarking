# Irregular MPI Benchmarking and Statistical Analysis
Collection of scripts to perform analysis and on parameters extracted from various mini-apps.

Please read the following for descriptions on data formats, analysis scripts, and build instructions for the benchmark included in this repository. 

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
For an example parameter file, see `data/example_data/example_clamr_parameter.txt`.
You can run any of the scripts in the repository on this parameter file to see how it behaves.

## Spack Repository
If you are wanting to collect data yourself, you can install the instrumented versions of the following applications via a private Spack repo:
- CLAMR
- Cabana

To install these packages via Spack, you should run the following commands (NOTE: [REPO_ROOT] refers to the relative path to this repository on your filesystem):

```bash
# list all repositories currently available on your system
spack repo list

# add the new repository
spack repo add [REPO_ROOT]/instrumented-repo/

# ensure that Spack installed the repository and that it is listed
spack repo list

# Ensure that the "instrumented" version is listed as a valid version for Cabana (as a test)
spack spec cabana@instrumented
```

### Updating an Installation
Because of the way that Spack manages versions (and how the instrumented versions of these packages are not assigned a version number), you will need to uninstall and reinstall a package to get changes made to it.
Lets use Cabana as an example again.
In the following example, you have installed CabanaMD (which depends on the instrumented Cabana), but want an updated version of the instrumented version of Cabana. 
To get this, run:
```bash
# removes the instrumented package and all packages depending on it
spack uninstall --dependents cabana@instrumented

# ensures that Spack will pull down a new copy
# instead of using a cached download
spack clean -a

# reinstall the instrumented cabana with new changes
spack install cabana@instrumented

# from here, you can install any package which depends on cabana such as...
spack install cabanamd ^cabana@instrumented
```

## Analysis

The `analysis.py` script performs statistical analysis on the parameters extracted from applications.
It does not require that you have the benchmark dependencies, only that you have the required python packages.

### Dependencies
Before running the `benchmark-runner.py` script, you will need the following:
- Python 3
- [matplotlib](https://pypi.org/project/matplotlib/)
- [scipy](https://pypi.org/project/scipy/)
- [numpy](https://pypi.org/project/numpy/)
- [fitter](https://pypi.org/project/fitter/)

### Running the Script
To run the script, you will use the following command:

```bash
$ python3 analysis.py --help
usage: analysis.py [-h] [-r [RPATH]] [--bin-count [BIN_COUNT]] [-c] [--disable-distribution-fitting] [param_path]

positional arguments:
  param_path            Specify path to file where parameter data is stored

options:
  -h, --help            show this help message and exit
  -r [RPATH]            Overrides where results are stored
  --bin-count [BIN_COUNT]
                        Specify number of bins for empirical distribution fitting Can be a numerical value or "auto" to set the value dynamically.
  -c, --clean           Removes previously generated files
  --enable-distribution-fitting
                        Enables lengthy process of fitting parameters to best distribution
```
You must specify a path to a file where your parameter data is stored.
This script generates a results directory named the same name as the parameter data file name. 
Within this directory, graphs of the parameter data will be generated alongside a `BENCHMARK_CONFIG` file. 
This file is used to quickly pass statistical data into the benchmark for recreation, so do not delete or modify this file without knowing what it is that you are doing. 
If this occurs, you can simply regenerate the file (which can take a while, please see the following sections for notices on performance of this script).

It is **HIGHLY** recommended that you set the `--bin-count` flag to something sensible like 10 or 20.
Leaving it on auto will potentially create a huge amount of bins.
This is not a problem and can be handled just fine in the benchmark and by the script, but will cause significant processing time before the analysis and graphs are generated. 

## Run the Benchmark

### Dependencies
To build/run the benchmark, you will need:
- C/C++ compiler
- MPI
- CMake

Optionally, if your system supports the following options, you can build for them:
- CUDA
- OpenMP
- OpenCL

### Building

You should first build the benchmark with the following commands (as usual, [REPO_ROOT] is the relative path to this repository on your system):
```bash
mkdir build
cd build
cmake [REPO_ROOT]
make -j $(nproc)
```

### Running
Once the benchmark has been built, you can run the benchmark with the following command:

```bash
mpirun ./benchmark
```

To run the benchmark on a `BENCHMARK_CONFIG` (required for using an empirical distribution), you can pass in the path with a `-f [PATH]` flag. 
You will also need to specify the correct distribution via the `-d` distribution flag. 
For a full list of options, you can run `--help` to get the following:
```
mpirun -np 1 ./benchmark --help
usage: ./benchmark [-t typesize] [-I samples] [-i iterations] [-n neighbors] [-o owned] [-r remote] [-b blocksize] [-s stride] [-S seed] [-m memspace]

[ -f filepath       ]	specify the path to the BENCHMARK_CONFIG file
[ -t typesize       ]	specify the size of the variable being sent (in bytes)
[ -I samples        ]	specify the number of random samples to generate
[ -i iterations     ]	specify the number of updates each sample performs
[ -n neighbors      ]	specify the average number of neighbors each process communicates with
[ -N neighbors_stdv ]	specify the stdev number of neighbors each process communicates with
[ -o owned_avg      ]	specify average byte count for data owned per node
[ -O owned_stdv     ]	specify stdev byte count for data owned per node
[ -r remote_avg     ]	specify how average amount of data each process receives
[ -R remote_stdv    ]	specify how average amount of data each process receives
[ -b blocksize_avg  ]	specify average size of transmitted blocks
[ -B blocksize_stdv ]	specify average size of transmitted blocks
[ -s stride         ]	specify average size of stride
[ -T stride_stdv    ]	specify stdev size of stride
[ -S seed           ]	specify positive integer to be used as seed for random number generation (current time used as default)
[ -m memspace       ]	choose from: host, cuda, openmp, opencl
[ -d distribution   ]	choose from: gaussian (default), empirical
[ -u units          ]	choose from: a,b,k,m,g (auto, bytes, kilobytes, etc.)

NOTE: setting parameters for the benchmark such as (neighbors, owned, remote, blocksize, and stride)
      sets parameters to those values for the reference benchmark.
      Those parameters are then randomized for the irregular samples
      where the user-set parameters become averages for the random generation.
      Use the `--disable-irregularity` flag to only run the reference benchmark.
```

It is, of course, expected that you should update the `mpirun` command to better use and take advantage of your system's resources. 
This could include using Slurm for resource allocation and management. 
This README does not include how to accomplish that, however there shouldn't be any problems with such an approach. 
