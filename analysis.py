import argparse
import statistics
import os
import shutil
import math
from pathlib import Path
import matplotlib.pyplot as plt
from scipy.stats import shapiro
from model.parameter import Parameter
import numpy as np


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


def analysis_combined(params, results_dir="results", filename="0", param_names=[], DPI=800):
    title_font = {"family": "Serif", "weight": "normal", "size": 11}
    axes_font = {"family": "Serif", "weight": "normal", "size": 10}

    rows = len(params)
    cols = 3
    fig, axs = plt.subplots(rows, cols)
    for param_idx in range(0, len(params)):
        print("Making N-Owned Graph for: " + str(param_idx))
        ax = axs[param_idx, 0]
        ax.hist(params[param_idx].nowned, bins=20, color="#00416d")
        ax.set_title("N-Owned Size", **title_font)
        ax.set_xlabel("Size (bytes)", **axes_font)
        ax.set_ylabel("Frequency", **axes_font)
        ax.set_xbound(min(params[param_idx].nowned), max(params[param_idx].nowned))
        plt.tight_layout()
        ax.set_xticks(ax.get_xticks(), ax.get_xticklabels(), rotation=45, ha='right')

        print("Making N-Remote Graph for: " + str(param_idx))
        ax = axs[param_idx, 1]
        ax.hist(params[param_idx].nremote, bins=20, color="#00416d")
        ax.set_title("N-Remote Size", **title_font)
        ax.set_xlabel("Size (bytes)", **axes_font)
        ax.set_xbound(min(params[param_idx].nremote), max(params[param_idx].nremote))
        plt.tight_layout()
        ax.set_xticks(ax.get_xticks(), ax.get_xticklabels(), rotation=45, ha='right')
        
        print("Making N-Remote Graph for: " + str(param_idx))
        the_data, the_counts = np.unique(params[param_idx].comm_partners, return_counts=True)
        the_min = min(params[param_idx].comm_partners)
        the_max = max(params[param_idx].comm_partners)

        axs[param_idx, 2].bar(the_data, height=the_counts, color="#00416d")
        axs[param_idx, 2].set_title(" Comm-Partners", **title_font)
        axs[param_idx, 2].set_xlabel("Number of Partners", **axes_font)
        axs[param_idx, 2].set_xticks(ticks=np.arange(the_min, the_max+1), labels=np.arange(the_min, the_max+1), minor=False)

    plt.tight_layout()
    plt.savefig(results_dir + "/"+ filename, dpi=DPI)
    plt.clf()

def analysis(params, results_dir="results", file_name="", DPI=800):
    title_font = {"family": "Serif", "weight": "normal", "size": 16}
    axes_font = {"family": "Serif", "weight": "normal", "size": 12}

    if file_name != "":
        file_name += "\n"

    file_name = ""
    print("N-Owned: " + str(params.nowned_mean()))
    print("N-Owned stdev: " + str(params.nowned_stdev()))
    print("N-Owned dist: " + params.nowned_dist())
    plt.hist(params.nowned, bins=20, color="#00416d")
    plt.title(file_name + "Distribution of N-Owned Size", **title_font)
    plt.xlabel("Size (bytes)", **axes_font)
    plt.ylabel("Frequency", **axes_font)
    plt.tight_layout()
    plt.savefig(results_dir + "/nowned.png", dpi=DPI)
    plt.clf()

    print("N-Remote: " + str(params.nremote_mean()))
    print("N-Remote stdev: " + str(params.nremote_stdev()))
    print("N-Remote dist: " + params.nremote_dist())
    plt.hist(params.nremote, bins=20, color="#00416d")
    plt.title(file_name + "Distribution of N-Remote Size", **title_font)
    plt.xlabel("Size (bytes)", **axes_font)
    plt.ylabel("Frequency", **axes_font)
    plt.tight_layout()
    plt.savefig(results_dir + "/nremote.png", dpi=DPI)
    plt.clf()

    print("num_comm_partners: " + str(params.comm_partners_mean()) + "\n")
    #the_bins =  (max(params.comm_partners) - min(params.comm_partners)) + 1
    the_data, the_counts = np.unique(params.comm_partners, return_counts=True)

    plt.bar(the_data, height=the_counts, color="#00416d")
    plt.title(file_name + "Distribution of Comm-Partners Count", **title_font)
    plt.xlabel("Number of Partners", **axes_font)
    plt.ylabel("Frequency", **axes_font)
    plt.xticks(ticks=np.arange(min(params.comm_partners), max(params.comm_partners)+1), labels=np.arange(min(params.comm_partners), max(params.comm_partners)+1), minor=False)
    plt.tight_layout()
    plt.savefig(results_dir + "/comm_partners.png", dpi=DPI)
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
        plt.tight_layout()
        plt.savefig(results_dir + "/block_sizes.png", dpi=DPI)
        plt.clf()

    print("Stride: " + str(params.stride_mean()) + "\n")
    print("Stride stdev: " + str(params.stride_stdev()) + "\n")
    print("Stride distribution: " + params.stride_dist() + "\n")
    plt.hist(params.stride, bins=20, color="#00416d")
    plt.title(file_name + "Distrbution of Stride Size", **title_font)
    plt.xlabel("Size (bytes)", **axes_font)
    plt.ylabel("Frequency", **axes_font)
    plt.tight_layout()
    plt.savefig(results_dir + "/stride.png", dpi=DPI)
    plt.clf()

    print("updates_per_setup: " + str(params.updates_per_setup_mean()))
    print("updates_per_setup stdev: " + str(params.updates_per_setup_stdev()))
    print("updates_per_setup distribution: " + params.updates_per_setup_dist())


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        dest="param_path",
        action="store",
        nargs="+",
        type=argparse.FileType("r"),
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

    if args.bin_count != "auto":
        if not args.bin_count.isnumeric():
            sys.exit("Error: the bin-count argument must be an integer or auto")

    all_results = []
    all_params  = []
    param_names =[]
    for input_file in args.param_path:
        # bootstrap results directory
        param_names.append(input_file.name)
        file_name = input_file.name.split("/")[-1].split(".")[0]
        procs = input_file.name.split("/")[-1].split("_")[-2]
        print("Procs: " + procs)
        print("Analyzing: " + file_name)
        results = os.path.join(args.rpath, file_name)
        bootstrap_results(results, clean=args.clean)
        all_results.append(results)

        # tries to read file containing parameter data
        # fails if file does not exist, cannot be read, etc.
        try:
            with open(input_file.name, "r") as param_file:
                print("Parsing: " + input_file.name)
                param_output = param_file.readlines()
        except:
            print(
                "Error: could not read parameter output file: %s. Are you sure that file exists?" % input_file.name
            )

        params = Parameter(
            param_output,
            fit_distribution=args.enable_distribution_fitting,
            results_dir=None,
            bin_count=args.bin_count,
        )
        all_params.append(params)

    # generate distribution plots
    analysis_combined(all_params, filename=procs, param_names=param_names)
        
