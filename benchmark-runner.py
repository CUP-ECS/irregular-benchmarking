import sys
import subprocess
import argparse
import statistics
import os
import glob
import shutil
import math
import platform

def process_params(clamr_output):
    """
    clamr_output: output from clamr executable which is being processed
    """
    nowned = []
    nremote = []
    comm_partners = []
    indices_needed = []
    nsizes = []
    block_sizes = []

    for line in clamr_output:
        if "nowned" in line:
            line = line.split("-")
            if line[1].strip().isdigit():
                nowned.append(int(line[1]))
        elif "nremote" in line:
            line = line.split("-")
            if line[1].strip().isdigit():
                nremote.append(int(line[1]))
        elif "num_comm_partners" in line:
            line = line.split("-")
            if line[1].strip().isdigit():
                comm_partners.append(int(line[1]))
        elif "indices_needed" in line:
            line = line.split("-")[1].strip().split(" ")

            # initialize nested lists of indices_needed
            # if indices_needed is empty
            if len(indices_needed) == 0:
                indices_needed = [[]] * len(line)

            # append value to nested list
            for i, val in enumerate(line):
                indices_needed[i].append(int(line[i]))
        elif "nsizes" in line:
            line = line.split("-")
            line = line[1].strip().split(" ")

            # initialize nested lists of indices_needed
            # if indices_needed is empty
            if len(nsizes) == 0:
                nsizes = [[]] * len(line)

            # append value to nested list
            for i, val in enumerate(line):
                nsizes[i].append(int(line[i]))
        elif "blocksize" in line:
            line = line.split("-")
            if line[1].strip().isdigit():
                block_sizes.append(int(line[1]))
        elif "PARAM:" in line:
            print(line)

    print("nowned: " + str(round(statistics.mean(nowned))))
    print("nowned stdev: " + str(round(statistics.stdev(nowned))))
    print("nremote: " + str(round(statistics.mean(nremote))))
    print("nremote stdev: " + str(round(statistics.stdev(nremote))))
    print("num_comm_partners: " + str(round(statistics.mean(comm_partners))))

    if len(block_sizes) == 0:
        print("Error: Blocksize was not printed")
    else:
        print("block_sizes: " + str(round(statistics.mean(block_sizes))))
        print("block_sizes stdev: " + str(round(statistics.stdev(block_sizes))))

    return [
        round(statistics.mean(nowned)),
        round(statistics.stdev(nowned)),
        round(statistics.mean(nremote)),
        round(statistics.stdev(nremote)),
        round(statistics.mean(comm_partners)),
        round(statistics.mean(block_sizes)),
        round(statistics.stdev(block_sizes))
    ]

def run_benchmark_with_params(nodes, nowned_avg, nowned_stdv,
                              nremote_avg, nremote_stdv, comm_partners,
                              block_size_avg=3, block_size_stdv=3):
    if "quartz" in platform.node():
        cmd = ['srun', './l7_update_perf',
               '-o', str(nowned_avg), '-r', str(nremote_avg),
               '-n', str(comm_partners), '-b', str(block_size_avg), '-I', str(10)]
    elif "lassen" in platform.node():
        cmd = ['lrun', '-N', nodes, '-T', '40', './l7_update_perf',
               '-o', str(nowned_avg), '-r', str(nremote_avg),
               '-n', str(comm_partners), '-b', str(block_size_avg), '-I', str(10)]
    else:
        print("Error: unsupported system")
        exit()
    output = subprocess.run(cmd, capture_output=False, encoding='UTF-8').stdout

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-n',
                        dest='nodes',
                        default=1,
                        action='store',
                        nargs='?',
                        type=int,
                        help='Specify node count')
    nodes = str(parser.parse_args().nodes)

    try:
        clamr_outfile = open("CLAMR.out","r")
        clamr_output = clamr_outfile.readlines()
    except:
        print("Error: could not read CLAMR output from file.")
        exit()

    params = process_params(clamr_output)

    # blocksize is not always reported
    # this ensures that, if not reported, parameters are not passed
    if len(params) > 4:
        run_benchmark_with_params(nodes=nodes, nowned_avg=params[0],
                              nowned_stdv=params[1], nremote_avg=params[2],
                              nremote_stdv=params[3], comm_partners=params[4],
                              block_size_avg=params[5], block_size_stdv=params[5])
    else:
        run_benchmark_with_params(nodes=nodes, nowned_avg=params[0],
                              nowned_stdv=params[1], nremote_avg=params[2],
                              nremote_stdv=params[3], comm_partners=params[4])

