import sys
import subprocess
import argparse
import statistics
import os
import glob
import shutil
import math
import platform
from pathlib import Path


class Parameter:
    def __init__(self, param_file):
        self.nowned = []
        self.nremote = []
        self.blocksize = []
        self.stride = []
        self.comm_partners = []

        for param_line in param_file:
            line = param_line.lower()
            if "nowned" in line:
                line = line.split("-")
                if line[1].strip().isdigit():
                    self.nowned.append(int(line[1]))
            elif "nremote" in line:
                line = line.split("-")
                if line[1].strip().isdigit():
                    self.nremote.append(int(line[1]))
            elif "num_comm_partners" in line:
                line = line.split("-")
                if line[1].strip().isdigit():
                    self.comm_partners.append(int(line[1]))
            elif "blocksize" in line:
                line = line.split("-")
                if line[1].strip().isdigit():
                    self.blocksize.append(int(line[1]))
            elif "stride" in line:
                line = line.split("-")
                if line[1].strip().isdigit():
                    self.stride.append(int(line[1]))

    def nowned_mean(self):
        if len(self.nowned) == 0:
            return 0
        else:
            return round(statistics.mean(self.nowned))

    def nowned_stdev(self):
        if len(self.nowned) == 0:
            return 0
        else:
            return round(statistics.stdev(self.nowned))

    def nremote_mean(self):
        if len(self.nremote) == 0:
            return 0
        else:
            return round(statistics.mean(self.nremote))

    def nremote_stdev(self):
        if len(self.nremote) == 0:
            return 0
        else:
            return round(statistics.stdev(self.nremote))

    def blocksize_mean(self):
        if len(self.blocksize) == 0:
            return 0
        else:
            return round(statistics.mean(self.blocksize))

    def blocksize_stdev(self):
        if len(self.blocksize) == 0:
            return 0
        else:
            return round(statistics.stdev(self.blocksize))

    def stride_mean(self):
        if len(self.stride) == 0:
            return 0
        else:
            return round(statistics.mean(self.stride))

    def stride_stdev(self):
        if len(self.stride) == 0:
            return 0
        else:
            return round(statistics.stdev(self.stride))

    def comm_partners_mean(self):
        if len(self.comm_partners) == 0:
            return 0
        else:
            return round(statistics.mean(self.comm_partners))


def bootstrap_benchmark(stage_dir=Path(".benchmark_stage")):
    """
    Bootstraps benchmark binary
    """
    print("Bootstrapping benchmark...")

    # delete previously created stage if it exists
    if os.path.exists(stage_dir):
        shutil.rmtree(stage_dir)

    # create new stage directory
    os.mkdir(stage_dir)

    # run cmake step of build stage
    subprocess.run(
        ["cmake", ".."], capture_output=True, encoding="UTF-8", cwd=stage_dir
    )

    # run build stage
    subprocess.run(
        ["make", "-j", "8"], capture_output=True, encoding="UTF-8", cwd=stage_dir
    )

    # ensures benchmark was built properly
    if not os.path.isfile(os.path.join(stage_dir, "l7_update_perf")):
        print("\nError: could not bootstrap successfully")
        exit(-1)
    else:
        print("Benchmark bootstrapping complete.")


def bootstrap_results(results_dir=Path("results"), stage_dir=Path(".benchmark_stage")):
    """
    Setup where results are stored
    """
    # delete previously created results if exists
    if os.path.exists(results_dir):
        shutil.rmtree(results_dir)

    # create new results directory
    os.mkdir(results_dir)

    # copy benchmark
    try:
        shutil.copy(os.path.join(stage_dir, "l7_update_perf"), results_dir)
    except:
        raise SystemExit("Error: could not move benchmark binary to results directory")


def run_benchmark_with_params(params, results_dir="results/"):
    cmd = [
        "srun",
        "./l7_update_perf",
        "-o",
        str(params.nowned_mean()),
        "-r",
        str(params.nremote_mean()),
        "-n",
        str(params.comm_partners_mean()),
        "-b",
        str(params.blocksize_mean()),
        "-I",
        str(10),
    ]

    output = subprocess.run(
        cmd, capture_output=False, encoding="UTF-8", cwd=results_dir
    ).stdout


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        dest="param_path",
        action="store",
        nargs="?",
        type=str,
        help="Specify path to file where parameter data is stored",
    )
    parser.add_argument(
        "-b",
        dest="bpath",
        default=".benchmark_stage",
        action="store",
        nargs="?",
        type=str,
        required=False,
        help="Overrides where benchmark is built",
    )
    parser.add_argument(
        "-r",
        dest="rpath",
        default="results",
        action="store",
        nargs="?",
        type=str,
        help="Overrides where results are stored",
    )

    args = parser.parse_args()

    # parameter path must be specified
    if not args.param_path:
        raise SystemExit("Error: no path to parameter log file specified")

    # bootstraps benchmark from source
    bootstrap_benchmark(Path(args.bpath))

    # bootstrap results directory
    bootstrap_results(Path(args.rpath), Path(args.bpath))

    # tries to read file containing parameter data
    # fails if file does not exist, cannot be read, etc.
    try:
        param_file = open(args.param_path, "r")
        param_output = param_file.readlines()
        param_file.close()
    except:
        raise SystemExit(
            "Error: could not read parameter output file. Are you sure that file exists?"
        )

    # run analysis on parameter data
    params = Parameter(param_output)
    print("nowned: " + str(params.nowned_mean()))
    print("nowned stdev: " + str(params.nowned_stdev()))
    print("nremote: " + str(params.nremote_mean()))
    print("nremote stdev: " + str(params.nremote_stdev()))
    print("block_sizes: " + str(params.blocksize_mean()))
    print("block_sizes stdev: " + str(params.blocksize_stdev()))
    print("num_comm_partners: " + str(params.comm_partners_mean()))

    # run benchmark with parameter data
    run_benchmark_with_params(params, args.rpath)
