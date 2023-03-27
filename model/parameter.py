import statistics
import os
import numpy as np
import concurrent.futures
from fitter import Fitter, get_common_distributions, get_distributions


class Parameter:
    def __init__(self, param_file, fit_distribution=True, results_dir=None):
        self.nowned = []
        self.nremote = []
        self.blocksize = []
        self.stride = []
        self.comm_partners = []
        self.updates_per_setup = []
        self.fit_distribution = fit_distribution

        for param_line in param_file:
            # ensures we don't accidentally pick up
            # from regular program output
            if "PARAM" in param_line:
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
                elif "indices_needed" in line:
                    line = line.split("-")[1].split()
                    for index, index_value in enumerate(line):
                        if index < int(len(line) - 1):
                            self.stride.append(int(line[index + 1]) - int(line[index]))
                elif "setup called" in line:
                    self.updates_per_setup.append(0)
                elif "update called" in line:
                    self.updates_per_setup[-1] = self.updates_per_setup[-1] + 1

        if fit_distribution == True:
            self.nowned_distr = self._dist_test(self.nowned)
            self.nremote_distr = self._dist_test(self.nremote)
            self.blocksize_distr = self._dist_test(self.blocksize)
            self.stride_distr = self._dist_test(self.stride)
            self.comm_partners_distr = self._dist_test(self.comm_partners)
            self.updates_per_setup_distr = self._dist_test(self.updates_per_setup)

        if results_dir is not None:
            self.generate_file(results_dir)

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

    def nowned_dist(self):
        if self.fit_distribution == False:
            return "Unknown"
        return str(self.nowned_distr.get_best(method="sumsquare_error"))

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

    def nremote_dist(self):
        if self.fit_distribution == False:
            return "Unknown"
        return str(self.nremote_distr.get_best(method="sumsquare_error"))

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

    def blocksize_dist(self):
        if self.fit_distribution == False:
            return "Unknown"
        return str(self.blocksize_distr.get_best(method="sumsquare_error"))

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

    def stride_dist(self):
        if self.fit_distribution == False:
            return "Unknown"
        return str(self.stride_distr.get_best(method="sumsquare_error"))

    def comm_partners_mean(self):
        if len(self.comm_partners) == 0:
            return 0
        else:
            return round(statistics.mean(self.comm_partners))

    def comm_partners_stdev(self):
        if len(self.comm_partners) == 0:
            return 0
        else:
            return round(statistics.stdev(self.comm_partners))

    def comm_partners_dist(self):
        if self.fit_distribution == False:
            return "Unknown"
        return str(self.comm_partners_distr.get_best(method="sumsquare_error"))

    def updates_per_setup_mean(self):
        if len(self.updates_per_setup) == 0:
            return 0
        else:
            return round(statistics.mean(self.updates_per_setup))

    def updates_per_setup_stdev(self):
        if len(self.updates_per_setup) == 0:
            return 0
        else:
            return round(statistics.stdev(self.updates_per_setup))

    def updates_per_setup_dist(self):
        if self.fit_distribution == False:
            return "Unknown"
        return str(self.updates_per_setup_distr.get_best(method="sumsquare_error"))

    def _dist_test(self, param):
        f = Fitter(param)
        f.fit()
        return f

    def generate_file(self, results_dir):
        file_path = os.path.join(results_dir, "BENCHMARK_CONFIG")
        file = open(file_path, "w")
        file.write("BENCHMARK INPUT FILE\n")
        file.write("FORMAT IS ORDERED AS FOLLOWS:\n\n")
        file.write("PARAM: PARAM_NAME\n")
        file.write("BIN_COUNT: BIN_COUNT_VALUE\n")
        file.write("BIN_MIN, BIN_MAX, BIN_PROP, BIN_MEAN, BIN_STDEV\n\n")

        # nowned_bins = self.binify("nowned", self.nowned) + "\n"
        # nremote_bins = self.binify("nremote", self.nremote) + "\n"
        # blocksize_bins = self.binify("blocksize", self.blocksize) + "\n"
        # stride_bins = self.binify("stride", self.stride) + "\n"
        # comm_partners_bins = self.binify("comm_partners", self.comm_partners) + "\n"
        with concurrent.futures.ThreadPoolExecutor() as executor:
            futures = []
            futures.append(executor.submit(self.binify, "nowned", self.nowned))
            futures.append(executor.submit(self.binify, "nremote", self.nremote))
            futures.append(executor.submit(self.binify, "blocksize", self.blocksize))
            futures.append(executor.submit(self.binify, "stride", self.stride))
            futures.append(
                executor.submit(self.binify, "comm_partners", self.comm_partners)
            )

            results = [f.result() for f in concurrent.futures.as_completed(futures)]

        for result in results:
            file.write(result)
            file.write("\n")

        file.close()

    def binify(self, param_name, data, bin_count=None):
        """
        This function performs the initial data ingest for a
        specific parameter list.

        The function divides the data into bin_count bins
        (or calculates it automatically), calculates the min/max, mean,
        and stdev for each bin, and then assigns it a proportional value
        that represents the amount of data it holds in relation to the
        total list of data.

        Return: this function returns a multi-line string which is written
        to a file to be read by the benchmark
        """

        # sorts data (this makes binning data much easier later
        data.sort()

        # begin writing data to contents
        contents = "PARAM: " + str(param_name) + "\n"

        # calculate minimum and maximum values of nowned
        nowned_min = min(data)
        nowned_max = max(data)

        # calculate a good bin count assuming one is not specified
        if bin_count is None:
            bin_count = round(len(data) / 1000)

        # write the number of bins to ease the parsing in benchmark
        contents += "BIN_COUNT: " + str(bin_count) + "\n"

        # calculate the size of each bin based on the calculated binCount
        binSize = (nowned_max - nowned_min) / bin_count

        # identify the upper and lower bounds of each bin
        bins = []
        i = nowned_min
        for x in range(0, bin_count + 1):
            bins.append(round(i))
            i += binSize

        # iterate through each bin and find the statistical values
        # of data belonging to each specific bin
        for index, value in enumerate(bins):
            # don't perform bin calculations once on the last element
            if index != len(bins) - 1:
                mini_data = []
                bin_min = value
                bin_max = bins[index + 1]
                for parameter in data:
                    if parameter < bin_max:
                        # if value is less than bin_max, ensure it is
                        # larger than bin_min
                        if parameter >= bin_min:
                            mini_data.append(parameter)
                    else:
                        # because list is sorted, if value is not less
                        # than bin_max, no future value will be less
                        break

                # calculate what the proportion of the total data
                # belongs to this specific bin
                bin_prop = len(mini_data) / len(data)

                if len(mini_data) >= 2:
                    contents += (
                        str(bin_min)
                        + ", "
                        + str(bin_max)
                        + ", "
                        + f"{bin_prop:.20f}"
                        + ", "
                        + str(round(statistics.mean(mini_data)))
                        + ", "
                        + str(round(statistics.stdev(mini_data)))
                        + "\n"
                    )
                if len(mini_data) == 1:
                    # handles case where only 1 data point occurs in a bin
                    contents += (
                        str(bin_min)
                        + ", "
                        + str(bin_max)
                        + ", "
                        + f"{bin_prop:.20f}"
                        + ", "
                        + str(round(statistics.mean(mini_data)))
                        + ", "
                        + str(0)
                        + "\n"
                    )
                else:
                    # handles the case where no data belongs to a bin
                    contents += (
                        str(bin_min)
                        + ", "
                        + str(bin_max)
                        + ", "
                        + str(0.0)
                        + ", "
                        + str(0.0)
                        + ", "
                        + str(0.0)
                        + "\n"
                    )

        return contents
