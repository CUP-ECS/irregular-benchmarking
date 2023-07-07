#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// facilitates CLI-arg parsing
#include <getopt.h>

static int nneighbors = -1;
static int nneighbors_stdv = -1;
static int nowned = -1;
static int nowned_stdv = -1;
static int nremote = -1;
static int nremote_stdv = -1;
static int blocksz = -1;
static int blocksz_stdv = -1;
static int stride = -1;
static int stride_stdv = -1;
static char * filepath = NULL;

static struct option long_options[] = {
    /* These options set a flag. */
    {"help",           no_argument, 0, 'h'},
    {"config_file",    required_argument, 0, 'f'},
    {0, 0, 0, 0}
};

// offers a help string to define parameters in the CLI
void usage(char *exename)
{
    fprintf(stderr, "usage: %s \n\n use `--help` flag for more detailed instructions \n", exename);
    exit(-1);
}

// longer version of the usage function
// triggered on `--help` flag invocation
void usage_long(char *exename) {
    fprintf(stdout, "usage: %s \n\n", exename);
    exit(0);
}

void parse_config_file() {
    char* params[] = { "nowned",
                       "nremote",
                       "blocksize",
                       "stride",
                       "comm_partners" };

    for (int index = 0; index < (sizeof(params) / sizeof(params[0])); index++) {
        char* param = params[index];

        char param_key[25] = "PARAM: ";
        strcat(param_key, param);

        FILE* fp = fopen(filepath, "r");
        if (fp == NULL) {
            fprintf(stderr, "Error: Unable to open file %s\n", filepath);
            exit(1);
        }

        char* line = NULL;
        size_t linecap = 0;
        ssize_t linelen;

        // iterates through lines in the file until we get to the line
        // that contains the parameter we're trying to generate for
        while ((linelen = getline(&line, &linecap, fp)) != -1) {
            // remove trailing new line to better do comparison
            if (linelen > 0 && line[linelen-1] == '\n') {
                line[linelen-1] = '\0';
            }

            // if the correct parameter is found,
            // stop iterating through the file
            if (strcmp(param_key, line) == 0) {
                break;
            }
        }

        // iterates through non-bin data points
        for (int i = 0; i < 5; i++) {
            // gets the next line which contains the BIN_COUNT data
            linelen = getline(&line, &linecap, fp);

            // remove trailing new line to better do comparison
            if (linelen > 0 && line[linelen-1] == '\n') {
                line[linelen-1] = '\0';
            }

            // gets the name of the data point being read
            char* token = strtok(line, ":");

            // writes the correct token value to the correct variable from file input
            if (strcmp(token, "MEAN") == 0) {
                // puts the mean value in the correct parameter spot
                // based on current parameter choice
                if (strcmp(param, "nowned") == 0) {
                    nowned = atoi(strtok(NULL, " "));
                } else if (strcmp(param, "nremote") == 0) {
                    nremote = atoi(strtok(NULL, " "));
                } else if (strcmp(param, "blocksize") == 0) {
                    blocksz = atoi(strtok(NULL, " "));
                } else if (strcmp(param, "stride") == 0) {
                    stride = atoi(strtok(NULL, " "));
                } else if (strcmp(param, "comm_partners") == 0) {
                    nneighbors = atoi(strtok(NULL, " "));
                }
            } else if (strcmp(token, "STDEV") == 0) {
                // puts the stdev value in the correct parameter spot
                // based on current parameter choice
                if (strcmp(param, "nowned") == 0) {
                    nowned_stdv = atoi(strtok(NULL, " "));
                } else if (strcmp(param, "nremote") == 0) {
                    nremote_stdv = atoi(strtok(NULL, " "));
                } else if (strcmp(param, "blocksize") == 0) {
                    blocksz_stdv = atoi(strtok(NULL, " "));
                } else if (strcmp(param, "stride") == 0) {
                    stride_stdv = atoi(strtok(NULL, " "));
                } else if (strcmp(param, "comm_partners") == 0) {
                    nneighbors_stdv = atoi(strtok(NULL, " "));
                }
            }
        }
    }
}

void parse_arguments(int argc, char **argv)
{
    int c;
    bool config_file_used = false;
    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, ":h:f:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'f':
                // used to load BENCHMARK_CONFIG filepath
                filepath = optarg;

                if (access(filepath, F_OK) == -1) {
                    printf("ERROR: the specified filepath doesn't exist, exiting...\n");
                    exit(-1);
                }
                config_file_used = true;
                break;
            case 'h':
                usage_long(argv[0]);
                break;
            case '?':
                usage(argv[0]);
                break;
        }
    }

    /* parses the config file to set default mean & stdev values for:
     * - nowned
     * - nremote
     * - blocksize
     * - stride
     * - comm_partners
    */
    parse_config_file();
}

// implementation of the following
// https://en.wikipedia.org/wiki/Box–Muller_transform
int gauss_dist(double mean, double stdev) {
    // generates two random numbers that form the seeds
    // of the transform
    double u1, u2, r, theta;
    int generated = -1;

    while (generated <= 0) {
        u1 = (double)rand() / RAND_MAX;
        u2 = (double)rand() / RAND_MAX;

        // generates the R and Theta values from the above
        // documentation
        r = sqrt(-2.*log(u1));
        theta = (2*M_PI*u2);

        // an additional number can be generated in the
        // same distribution using the alternate form
        // ((r*sin(theta)) * stdev) + mean

        generated = round(((r*cos(theta)) * stdev ) + mean);
    }
    return generated;
}


// approx. implementation of the following for empirical distributions
// https://commons.apache.org/proper/commons-math/javadocs/api-3.6.1/org/apache/commons/math3/random/EmpiricalDistribution.html
//
// NOTE: this has been customized for use within this benchmark
//       and much of the statistical analysis required has been
//       precomputed for ease of use. You should not take the following
//       as a faithful and self-contained reproduction of the above link.
int empirical_dist(char param[], FILE* fp) {
    // Reset the file pointer to the start of the file
    fseek(fp, 0, SEEK_SET);

    // set the parameter string to look for.
    char param_key[25] = "PARAM: ";
    strcat(param_key, param);

    // stores the current read line and
    // some auxiliary information that assists in parsing the file
    char* line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    // iterates through lines in the file until we get to the line
    // that contains the parameter we're trying to generate for
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        // remove trailing new line to better do comparison
        if (linelen > 0 && line[linelen-1] == '\n') {
            line[linelen-1] = '\0';
        }

        // if the correct parameter is found,
        // stop iterating through the file
        if (strcmp(param_key, line) == 0) {
            break;
        }
    }

    int binCount;
    int dataMin;
    int dataMax;
    int dataMean;
    int dataStdev;

    // Iterates through non-bin data points
    // and collect information about the data and number of bins
    for (int i = 0; i < 5; i++) {
        // gets the next line which contains the BIN_COUNT data
        linelen = getline(&line, &linecap, fp);

        // remove trailing new line to better do comparison
        if (linelen > 0 && line[linelen-1] == '\n') {
            line[linelen-1] = '\0';
        }

        // gets the name of the data point being read
        char* token = strtok(line, ":");

        // writes the correct token value to the correct variable from file input
        if (strcmp(token, "BIN_COUNT") == 0) {
            binCount = atoi(strtok(NULL, " "));
        } else if (strcmp(token, "MIN") == 0) {
            dataMin = atoi(strtok(NULL, " "));
        } else if (strcmp(token, "MAX") == 0) {
            dataMax = atoi(strtok(NULL, " "));
        } else if (strcmp(token, "MEAN") == 0) {
            dataMean = atoi(strtok(NULL, " "));
        } else if (strcmp(token, "STDEV") == 0) {
            dataStdev = atoi(strtok(NULL, " "));
        } else {
            printf("Error: reading data file failed, invalid data point found: %s\n", token);
            exit(1);
        }
    }

    // do a quick sanity check to ensure that the data doesn't only
    // fall into a single bin
    if (dataMin == dataMax) {
        return dataMin; // could also return data max, it is arbitrary
    }

    // chose a uniformly distributed value between (0,1)
    double binSelection = (double)rand() / RAND_MAX;


    // used to track if we have checked a bin
    // if we have/are, the bin's proportion of the total dataset
    // is added to the binSum.
    // If the binSelection falls under the binSum, it is said
    // to be in the bin and we can use that bin's data to generate values
    double binSum = 0.0;

    // iterates through the bins for a parameter to identity
    // which one should be chosen based on the random binSelection
    // value chosen above
    for (int i = 0; i < binCount; i++) {
        // gets the next line which contains the BIN_COUNT data
        linelen = getline(&line, &linecap, fp);

        // remove trailing new line to better do comparison
        if (linelen > 0 && line[linelen-1] == '\n') {
            line[linelen-1] = '\0';
        }

        // stores data from the current bin being checked
        int binMin;
        int binMax;
        int binMean;
        int binStdev;

        // gets the bin minimum data point
        char* token = strtok(line, ",");
        binMin = atoi(token);

        // gets the bin maximum data point
        token = strtok(NULL, ",");
        binMax = atoi(token);

        // gets the binProp value and adds it to the binSum
        // NOTE: I don't think this will ever chose a bin
        //       that has a prop value of 0, as binSum would remain
        //       the same and would not trigger the binSelection check
        token = strtok(NULL, ",");
        binSum += strtod(token, NULL);

        // if so, then the selection is in the most
        // recently searched bin, so we can proceed
        // to generating a random value
        // also triggers if on the last bin and no value has been selected
        //printf("Progress: searching for bin %d/%d\n", i, binCount);
        if (binSelection < binSum || i == (binCount-1)) {
            // printf("Progress: found bin\n");
            // if the bin is the correct one,
            // get the mean of the data points
            // for this bin
            token = strtok(NULL, ",");
            binMean = atoi(token);

            // if the bin is the correct one,
            // get the standard deviation of the
            // data points in this bin
            token = strtok(NULL, ",");
            binStdev = atoi(token);

            // return a normally distributed value with the
            // mean and standard deviation from the
            // chosen bin
            return gauss_dist(binMean, binStdev);
        }

    }

    // if we have somehow made it to this point
    // without generating a value, we should throw an error
    printf("Error: empirical value could not be generated due to an unknown error.\n");
    printf("Parameter type being generated: %s\n", param);
    printf("Config file which could have caused this error: %s\n", filepath);
    printf("If you encounter a bug which causes this for any reason, please open an issue on Github\n");
    printf("Here is additional debugging information\n");
    printf("\tbinSelection: %f\n", binSelection);
    printf("\tbinSum: %f\n", binSum);
    exit(1);
}


int main(int argc, char *argv[])
{
    srand(time(NULL));

    // parse CLI arguments
    parse_arguments(argc, argv);

    // attempts to open the file pointer
    // and ensure that the file can be read
    // exits if fails
    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", filepath);
        exit(1);
    }

    const char* strings[] = { "nowned", "nremote", "blocksize", "comm_partners", "stride" };
    const int numStrings = sizeof(strings) / sizeof(strings[0]);
    const int iterations = 100000;

    int i, j;
    for (i = 0; i < iterations; i++) {
        // Calculate the percentage completion
        float progress = ((float)i / iterations) * 100;

        // Print the progress
        printf("Progress: %.2f%%\n", progress);

        for (j = 0; j < numStrings; j++) {
            // printf("Progress generating: %s", strings[j]);
            printf("PARAM: %s - %d\n", strings[j], empirical_dist(strings[j], fp));
        }
    }

    // close file as reading is no longer required
    fclose(fp);



   return 0;
}
