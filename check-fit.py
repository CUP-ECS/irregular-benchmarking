import argparse
from pathlib import Path
from scipy.stats import ks_2samp
from model.parameter import Parameter


def print_stats(param, statistic, p_value, alpha):
    print(f"Checking {param} Fit")
    print(f"\tKolmogorov-Smirnov Statistic: {statistic}")
    print(f"\tP-Value: {p_value}")
    if p_value >= alpha:
        print("\tThis is a good fit!\n")
    else:
        print("\tThis is a bad fit!")


def check_fit(observed_params, generated_params):
    if observed_params.nowned and generated_params.nowned:
        # max_gen = max(generated_params.nowned)
        # min_gen = min(generated_params.nowned)
        # observed_nowned = [x for x in observed_params.nowned if x < max_gen]
        # observed_nowned = [x for x in observed_nowned if x > min_gen]
        # if max_gen >= max(observed_nowned):
        #     print("Removed Upper Outliers")
        # if min_gen <= min(observed_nowned):
        #     print("Removed Lower Outliers")

        # Perform the Kolmogorov-Smirnov test for N-Owned
        statistic, p_value = ks_2samp(observed_params.nowned, generated_params.nowned)
        print_stats("N-Owned", statistic, p_value, 0.05)
        print(f"\tObserved Data Count: {len(observed_params.nowned)}")
        print(f"\tObserved Data Count Min: {min(observed_params.nowned)}")
        print(f"\tObserved Data Count Max: {max(observed_params.nowned)}")
        print(f"\tGenerated Data Count: {len(generated_params.nowned)}")
        print(f"\tGenerated Data Count Min: {min(generated_params.nowned)}")
        print(f"\tGenerated Data Count Max: {max(generated_params.nowned)}\n")
    else:
        print("Skipped N-Owned because parameter is empty.\n")

    if observed_params.nremote and generated_params.nremote:
        # Perform the Kolmogorov-Smirnov test for N-Remote
        statistic, p_value = ks_2samp(observed_params.nremote, generated_params.nremote)
        print_stats("N-Remote", statistic, p_value, 0.05)
        print(f"\tObserved Data Count: {len(observed_params.nremote)}")
        print(f"\tObserved Data Count Min: {min(observed_params.nremote)}")
        print(f"\tObserved Data Count Max: {max(observed_params.nremote)}")
        print(f"\tGenerated Data Count: {len(generated_params.nremote)}")
        print(f"\tGenerated Data Count Min: {min(generated_params.nremote)}")
        print(f"\tGenerated Data Count Max: {max(generated_params.nremote)}\n")
    else:
        print("Skipped N-Remote because parameter is empty.\n")

    if observed_params.blocksize and generated_params.blocksize:
        # Perform the Kolmogorov-Smirnov test for Blocksize
        statistic, p_value = ks_2samp(
            observed_params.blocksize, generated_params.blocksize
        )
        print_stats("Blocksize", statistic, p_value, 0.05)
        print(f"\tObserved Data Count: {len(observed_params.blocksize)}")
        print(f"\tObserved Data Count Min: {min(observed_params.blocksize)}")
        print(f"\tObserved Data Count Max: {max(observed_params.blocksize)}")
        print(f"\tGenerated Data Count: {len(generated_params.blocksize)}")
        print(f"\tGenerated Data Count Min: {min(generated_params.blocksize)}")
        print(f"\tGenerated Data Count Max: {max(generated_params.blocksize)}\n")
    else:
        print("Skipped Blocksize because parameter is empty.\n")

    if observed_params.stride and generated_params.stride:
        # Perform the Kolmogorov-Smirnov test for Stride
        statistic, p_value = ks_2samp(observed_params.stride, generated_params.stride)
        print_stats("Stride", statistic, p_value, 0.05)
        print(f"\tObserved Data Count: {len(observed_params.stride)}")
        print(f"\tObserved Data Count Min: {min(observed_params.stride)}")
        print(f"\tObserved Data Count Max: {max(observed_params.stride)}")
        print(f"\tGenerated Data Count: {len(generated_params.stride)}")
        print(f"\tGenerated Data Count Min: {min(generated_params.stride)}")
        print(f"\tGenerated Data Count Max: {max(generated_params.stride)}\n")

    else:
        print("Skipped Stride because parameter is empty.\n")

    if observed_params.comm_partners and generated_params.comm_partners:
        # Perform the Kolmogorov-Smirnov test for num_comm_partners
        statistic, p_value = ks_2samp(
            observed_params.comm_partners, generated_params.comm_partners
        )
        print_stats("comm_partners", statistic, p_value, 0.05)
        print(f"\tObserved Data Count: {len(observed_params.comm_partners)}")
        print(f"\tObserved Data Count Min: {min(observed_params.comm_partners)}")
        print(f"\tObserved Data Count Max: {max(observed_params.comm_partners)}")
        print(f"\tGenerated Data Count: {len(generated_params.comm_partners)}")
        print(f"\tGenerated Data Count Min: {min(generated_params.comm_partners)}")
        print(f"\tGenerated Data Count Max: {max(generated_params.comm_partners)}\n")

    else:
        print("Skipped num_comm_partners because parameter is empty.\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        dest="observed_data",
        action="store",
        nargs="?",
        type=str,
        help="Specify path to file where parameter data from an application is stored",
    )

    parser.add_argument(
        dest="generated_data",
        action="store",
        nargs="?",
        type=str,
        help="Specify path to file where parameter data generated from benchmark is stored",
    )

    args = parser.parse_args()

    # parameter path must be specified
    if not args.observed_data or not args.generated_data:
        raise SystemExit("Error: must specify two datafiles")

    # tries to read file containing parameter data
    # fails if file does not exist, cannot be read, etc.
    try:
        observed_data_file = open(args.observed_data, "r")
        observed_data = observed_data_file.readlines()
        observed_data_file.close()

        generated_data_file = open(args.generated_data, "r")
        generated_data = generated_data_file.readlines()
        generated_data_file.close()
    except:
        raise SystemExit(
            "Error: could not read one of the two files. Are you sure that both files exists?"
        )

    # run analysis on parameter data
    observed_params = Parameter(observed_data, fit_distribution=False)

    generated_params = Parameter(generated_data, fit_distribution=False)

    # generate distribution plots
    check_fit(observed_params, generated_params)
