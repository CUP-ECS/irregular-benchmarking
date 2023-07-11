# fit an empirical cdf to a bimodal dataset
import argparse
from model.parameter import Parameter
import matplotlib.pyplot as plt
from numpy.random import normal
from numpy import hstack
from statsmodels.distributions.empirical_distribution import ECDF

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

# use color-blind friendly plot colors.
plt.style.use('tableau-colorblind10')

# run analysis on parameter data
observed_params = Parameter(observed_data, fit_distribution=False)
generated_params = Parameter(generated_data, fit_distribution=False)

# Create a figure and subplots
fig, axs = plt.subplots(2, 2)

title_font = {"family": "Serif", "weight": "normal", "size": 12}
axes_font = {"family": "Serif", "weight": "normal", "size": 9}

# fit a CDF
observed_ecdf = ECDF(observed_params.nowned)
generated_ecdf = ECDF(generated_params.nowned)

# plot N-Remote ECDF
axs[0, 0].plot(observed_ecdf.x, observed_ecdf.y, linestyle='-', label="Application")
axs[0, 0].plot(generated_ecdf.x, generated_ecdf.y, linestyle='--', label="Benchmark")
axs[0, 0].legend(loc="lower right")
axs[0, 0].set_xlabel('Bytes', **axes_font)
axs[0, 0].set_ylabel('Cumulative Probability', **axes_font)
axs[0, 0].set_title('N-Owned', **title_font)

# fit a CDF
observed_ecdf = ECDF(observed_params.nremote)
generated_ecdf = ECDF(generated_params.nremote)

# Plot N-Remote ECDF
axs[0, 1].plot(observed_ecdf.x, observed_ecdf.y, linestyle='-', label="Application")
axs[0, 1].plot(generated_ecdf.x, generated_ecdf.y, linestyle='--', label="Benchmark")
axs[0, 1].legend(loc="lower right")
axs[0, 1].set_xlabel('Bytes', **axes_font)
axs[0, 1].set_ylabel('Cumulative Probability', **axes_font)
axs[0, 1].set_title('N-Remote', **title_font)

# fit a CDF
observed_ecdf = ECDF(observed_params.blocksize)
generated_ecdf = ECDF(generated_params.blocksize)

# Plot N-Remote ECDF
axs[1, 0].plot(observed_ecdf.x, observed_ecdf.y, linestyle='-', label="Application")
axs[1, 0].plot(generated_ecdf.x, generated_ecdf.y, linestyle='--', label="Benchmark")
axs[1, 0].legend(loc="lower right")
axs[1, 0].set_xlabel('Bytes', **axes_font)
axs[1, 0].set_ylabel('Cumulative Probability', **axes_font)
axs[1, 0].set_title('Blocksize', **title_font)

# fit a CDF
observed_ecdf = ECDF(observed_params.stride)
generated_ecdf = ECDF(generated_params.stride)

# Plot N-Remote ECDF
axs[1, 1].plot(observed_ecdf.x, observed_ecdf.y, linestyle='-', label="Application")
axs[1, 1].plot(generated_ecdf.x, generated_ecdf.y, linestyle='--', label="Benchmark")
axs[1, 1].legend(loc="lower right")
axs[1, 1].set_xlabel('Bytes', **axes_font)
axs[1, 1].set_ylabel('Cumulative Probability', **axes_font)
axs[1, 1].set_title('Stride', **title_font)

fig.suptitle("CLAMR and Benchmark Parameter Distribution Comparison", **title_font)

# Adjust the spacing between subplots
plt.tight_layout()

plt.savefig("../results/ecdf.png", dpi=1200)

# Display the figure
plt.show()
