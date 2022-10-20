import statistics


class Parameter:
    def __init__(self, param_file):
        self.nowned = []
        self.nremote = []
        self.blocksize = []
        self.stride = []
        self.comm_partners = []
        self.updates_per_setup = []

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
            elif "indices_needed" in line:
                line = line.split("-")[1].split()
                for index, index_value in enumerate(line):
                    if index < int(len(line) - 1):
                        self.stride.append(int(line[index + 1]) - int(line[index]))
            elif "setup called" in line:
                self.updates_per_setup.append(0)
            elif "update called" in line:
                self.updates_per_setup[-1] = self.updates_per_setup[-1] + 1

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

    def updates_per_setup_mean(self):
        if len(self.updates_per_setup) == 0:
            return 0
        else:
            return round(statistics.mean(self.updates_per_setup))
