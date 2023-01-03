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


def analysis(params, results_dir="results"):
    print("nowned: " + str(params.nowned_mean()))
    print("nowned stdev: " + str(params.nowned_stdev()))
    print("nowned dist: " + params.nowned_dist())
    plt.hist(params.nowned, bins=20)
    plt.title("Distribution of nowned Size")
    plt.xlabel("size (in bytes)")
    plt.ylabel("frequency")
    plt.savefig(results_dir + "/nowned.png")
    plt.clf()

    print("nremote: " + str(params.nremote_mean()))
    print("nremote stdev: " + str(params.nremote_stdev()))
    print("nremote dist: " + params.nremote_dist())
    plt.hist(params.nremote, bins=20)
    plt.title("Distribution of nremote Size")
    plt.xlabel("size (in bytes)")
    plt.ylabel("frequency")
    plt.savefig(results_dir + "/nremote.png")
    plt.clf()

    print("num_comm_partners: " + str(params.comm_partners_mean()) + "\n")
    plt.hist(params.comm_partners, bins=20)
    plt.title("comm_partners size")
    plt.xlabel("number of partners")
    plt.ylabel("frequency")
    plt.savefig(results_dir + "/comm_partners.png")
    plt.clf()

    if len(params.blocksize) == 0:
        print("Blocksize is not reported")
    else:
        print("blocksize: " + str(params.blocksize_mean()))
        print("blocksize stdev: " + str(params.blocksize_stdev()))
        print("nremote dist: " + params.blocksize_dist())
        plt.hist(params.blocksize, bins=20)
        plt.title("blocksizes")
        plt.xlabel("size (bytes)")
        plt.ylabel("frequency")
        plt.savefig(results_dir + "/block_sizes.png")
        plt.clf()

    print("stride: " + str(params.stride_mean()) + "\n")
    print("stride stdev: " + str(params.stride_stdev()) + "\n")
    print("stride distribution: " + params.stride_dist() + "\n")
    plt.hist(params.stride, bins=20)
    plt.title("stride size")
    plt.xlabel("stride size (bytes)")
    plt.ylabel("frequency")
    plt.savefig(results_dir + "/stride.png")
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
        "-c", "--clean", action="store_true", help="Removes previously generated files"
    )

    args = parser.parse_args()

    # parameter path must be specified
    if not args.param_path:
        raise SystemExit("Error: no path to parameter log file specified")

    # bootstrap results directory
    print(args.param_path.split("/")[-1].split(".")[0])
    results = os.path.join(args.rpath, args.param_path.split("/")[-1].split(".")[0])
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
    params = Parameter(param_output)

    # generate distribution plots
    analysis(params)
