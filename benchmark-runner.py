import subprocess
import argparse
import statistics
import os
import sys
import shutil
import math
from pathlib import Path
from model.parameter import Parameter


def identify_launcher(name=""):
    """
    Checks if launcher program is available.
    Returns when first launcher is found.
    """
    if name == "":
        possible_launchers = ["srun", "lrun", "mpirun"]
    else:
        possible_launchers = [name]

    for launcher in possible_launchers:
        if shutil.which(launcher) is not None:
            return launcher

    raise SystemExit("Error: valid launcher not found")


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
    if not os.path.isfile(os.path.join(stage_dir, "benchmark")):
        print("\nError: could not bootstrap successfully")
        exit(-1)
    else:
        print("Benchmark bootstrapping complete.")


def bootstrap_results(
    results_dir=Path("results"), stage_dir=Path(".benchmark_stage"), clean=False
):
    """
    Setup where results are stored
    """
    # delete previously created results if exists
    if os.path.exists(results_dir) and clean:
        shutil.rmtree(results_dir)

    # create new results directory
    if not os.path.exists(results_dir):
        os.mkdir(results_dir)

    # copy benchmark
    try:
        shutil.copy(os.path.join(stage_dir, "benchmark"), results_dir)
    except:
        raise SystemExit("Error: could not move benchmark binary to results directory")


def run_benchmark_with_params(params, results_dir="results/"):
    cmd = [
        identify_launcher(),
        "./benchmark",
        "-o",
        str(params.nowned_mean()),
        "-O",
        str(params.nowned_stdev()),
        "-r",
        str(params.nremote_mean()),
        "-R",
        str(params.nremote_stdev()),
        "-n",
        str(params.comm_partners_mean()),
        "-b",
        str(params.blocksize_mean()),
        "-B",
        str(params.blocksize_stdev()),
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
        "--bin-count",
        dest="bin_count",
        default="auto",
        action="store",
        nargs="?",
        type=str,
        required=False,
        help="""
             Specify number of bins for empirical distribution fitting\n
             Can be a numerical value or \"auto\" to set the value dynamically.
             """,
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
    parser.add_argument(
        "-R",
        "--reproduce",
        action="store_true",
        help="Changes benchmark behavior to reproduce communication pattern",
    )
    parser.add_argument(
        "-c",
        "--clean",
        action="store_true",
        help="Removes previously generated results",
    )

    parser.add_argument(
        "--disable-distribution-fitting",
        action="store_false",
        help="Disables lengthy process of fitting parameters to best distribution",
    )

    args = parser.parse_args()

    # parameter path must be specified
    if not args.param_path:
        raise SystemExit("Error: no path to parameter log file specified")

    if args.bin_count != "auto":
        if not args.bin_count.isnumeric():
            sys.exit("Error: the bin-count argument must be an integer or auto")

    # bootstraps benchmark from source
    bootstrap_benchmark(Path(args.bpath))

    # bootstrap results directory
    file_name = args.param_path.split("/")[-1].split(".")[0]
    print("Running based on: " + file_name)
    results = os.path.join(args.rpath, file_name)
    bootstrap_results(results, clean=args.clean)

    # tries to read file containing parameter data
    # fails if file does not exist, cannot be read, etc.
    try:
        print(args.param_path)
        param_file = open(args.param_path, "r")
        param_output = param_file.readlines()
        param_file.close()
    except:
        raise SystemExit(
            "Error: could not read parameter output file. Are you sure that file exists?"
        )

    # run analysis on parameter data
    params = Parameter(
        param_output,
        fit_distribution=args.disable_distribution_fitting,
        results_dir=results,
        bin_count=args.bin_count,
    )
    print("nowned: " + str(params.nowned_mean()))
    print("nowned stdev: " + str(params.nowned_stdev()))
    print("nremote: " + str(params.nremote_mean()))
    print("nremote stdev: " + str(params.nremote_stdev()))
    print("block_sizes: " + str(params.blocksize_mean()))
    print("block_sizes stdev: " + str(params.blocksize_stdev()))
    print("num_comm_partners: " + str(params.comm_partners_mean()))

    # run benchmark with parameter data
    run_benchmark_with_params(params, args.rpath)
