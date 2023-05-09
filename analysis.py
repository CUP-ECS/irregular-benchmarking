import argparse
import statistics
import os
import shutil
import math
from pathlib import Path
import matplotlib.pyplot as plt
from scipy.stats import shapiro
from model.parameter import Parameter


def bootstrap_results(results_dir=Path("results"), clean=False):
    """
    Setup where results are stored
    """
    # delete previously created results if exists
    if os.path.exists(results_dir) and clean:
        shutil.rmtree(results_dir)

    # create new results directory
    if not os.path.exists(results_dir):
        os.makedirs(results_dir, exist_ok=True)


def analysis(params, results_dir="results", file_name=""):
    title_font = {"family": "Serif", "weight": "normal", "size": 16}
    axes_font = {"family": "Serif", "weight": "normal", "size": 12}

    if file_name != "":
        file_name += "\n"

    file_name=""
    print("N-Owned: " + str(params.nowned_mean()))
    print("N-Owned stdev: " + str(params.nowned_stdev()))
    print("N-Owned dist: " + params.nowned_dist())
    plt.hist(params.nowned, bins=20, color="#00416d")
    plt.title(file_name + "Distribution of N-Owned Size", **title_font)
    plt.xlabel("Size (bytes)", **axes_font)
    plt.ylabel("Frequency", **axes_font)
    plt.savefig(results_dir + "/nowned.png", dpi=1200)
    plt.clf()

    print("N-Remote: " + str(params.nremote_mean()))
    print("N-Remote stdev: " + str(params.nremote_stdev()))
    print("N-Remote dist: " + params.nremote_dist())
    plt.hist(params.nremote, bins=20, color="#00416d")
    plt.title(file_name + "Distribution of N-Remote Size", **title_font)
    plt.xlabel("Size (bytes)", **axes_font)
    plt.ylabel("Frequency", **axes_font)
    plt.savefig(results_dir + "/nremote.png", dpi=1200)
    plt.clf()

    print("num_comm_partners: " + str(params.comm_partners_mean()) + "\n")
    plt.hist(params.comm_partners, bins=20, color="#00416d")
    plt.title(file_name + "Distribution of Comm-Partners Count", **title_font)
    plt.xlabel("Number of Partners", **axes_font)
    plt.ylabel("Frequency", **axes_font)
    plt.savefig(results_dir + "/comm_partners.png", dpi=1200)
    plt.clf()

    if len(params.blocksize) == 0:
        print("Blocksize is not reported")
    else:
        print("blocksize: " + str(params.blocksize_mean()))
        print("blocksize stdev: " + str(params.blocksize_stdev()))
        print("blocksize dist: " + params.blocksize_dist())
        plt.hist(params.blocksize, bins=20, color="#00416d")
        plt.title(file_name + "Distribution of Block Sizes", **title_font)
        plt.xlabel("Size (bytes)", **axes_font)
        plt.ylabel("Frequency", **axes_font)
        plt.savefig(results_dir + "/block_sizes.png", dpi=1200)
        plt.clf()

    print("Stride: " + str(params.stride_mean()) + "\n")
    print("Stride stdev: " + str(params.stride_stdev()) + "\n")
    print("Stride distribution: " + params.stride_dist() + "\n")
    plt.hist(params.stride, bins=20, color="#00416d")
    plt.title(file_name + "Distrbution of Stride Size", **title_font)
    plt.xlabel("Size (bytes)", **axes_font)
    plt.ylabel("Frequency", **axes_font)
    plt.savefig(results_dir + "/stride.png", dpi=1200)
    plt.clf()

    print("updates_per_setup: " + str(params.updates_per_setup_mean()))
    print("updates_per_setup stdev: " + str(params.updates_per_setup_stdev()))
    print("updates_per_setup distribution: " + params.updates_per_setup_dist())


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
        "-r",
        dest="rpath",
        default="results",
        action="store",
        nargs="?",
        type=str,
        help="Overrides where results are stored",
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
        "-c", "--clean", action="store_true", help="Removes previously generated files"
    )
    parser.add_argument(
        "--enable-distribution-fitting",
        action="store_true",
        help="enables lengthy process of fitting parameters to best distribution",
    )

    args = parser.parse_args()

    # parameter path must be specified
    if not args.param_path:
        raise SystemExit("Error: no path to parameter log file specified")

    if args.bin_count != "auto":
        if not args.bin_count.isnumeric():
            sys.exit("Error: the bin-count argument must be an integer or auto")

    # bootstrap results directory
    file_name = args.param_path.split("/")[-1].split(".")[0]
    print("Analyzing: " + file_name)
    results = os.path.join(args.rpath, file_name)
    bootstrap_results(results, clean=args.clean)

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
    params = Parameter(
        param_output,
        fit_distribution=args.enable_distribution_fitting,
        results_dir=results,
        bin_count=args.bin_count,
    )

    # generate distribution plots
    analysis(params, results, file_name)
