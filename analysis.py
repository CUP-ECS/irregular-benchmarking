import argparse
import statistics
import os
import shutil
import math
from pathlib import Path
import matplotlib.pyplot as plt
from scipy.stats import shapiro

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


def bootstrap_results(results_dir=Path("results"), clean=False):
    """
    Setup where results are stored
    """
    # delete previously created results if exists
    if os.path.exists(results_dir) and clean:
        shutil.rmtree(results_dir)

    # create new results directory
    if not os.path.exists(results_dir):
        os.mkdir(results_dir)


def analysis(params, results_dir="results"):
    print("nowned: " + str(params.nowned_mean()))
    print("nowned stdev: " + str(params.nowned_stdev()))
    if shapiro(params.nowned).pvalue > .05:
        print("Distribution: Normal\n")
    else:
        print("Distribution: Not Normal\n")
    plt.hist(params.nowned, bins=20)
    plt.title("nowned size")
    plt.xlabel("size (bytes)")
    plt.ylabel("frequency")
    plt.savefig(results_dir + "/nowned.png")
    plt.clf()

    print("nremote: " + str(params.nremote_mean()))
    print("nremote stdev: " + str(params.nremote_stdev()))
    if shapiro(params.nremote).pvalue > .05:
        print("Distribution: Normal\n")
    else:
        print("Distribution: Not Normal\n")
    plt.hist(params.nremote, bins=20)
    plt.title("nremote size")
    plt.xlabel("size (bytes)")
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
        if shapiro(params.blocksize).pvalue > .05:
            print("Distribution: Normal\n")
        else:
            print("Distribution: Not Normal\n")
        plt.hist(params.blocksize, bins=20)
        plt.title("blocksizes")
        plt.xlabel("size (bytes)")
        plt.ylabel("frequency")
        plt.savefig(results_dir + "/block_sizes.png")
        plt.clf()


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
    bootstrap_results(Path(args.rpath), clean=args.clean)

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
