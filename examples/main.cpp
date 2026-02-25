#include <iostream>

#include <fstream>

#include <map>

#include <string>

#include <nlohmann/json.hpp>

#include <iostream>

#include <map>

#include <random>

#include <vector>

#include <chrono>

#include <iostream>

#include <memory>

#include <set>

#include <type_traits>

#include <cstdlib>

#include <Cabana_Core.hpp>

#include <Kokkos_Core.hpp>

#include <fstream>

#include <string>

#include <algorithm>

#include <tclap/CmdLine.h>

#include <nlohmann/json.hpp>

#include <cstdlib>

#include <cmath>

#include <filesystem>

#include <stdio.h>

#include <time.h>

#include <string.h>

#include <unistd.h>

#include <stdbool.h>

#include <limits>

#include <cxxabi.h>

#include <mcheck.h>

#define PRINT_LINE()                         \
do {                                    \
printf("Line: start %d\n", __LINE__);     \
fflush(stdout);                     \
MPI_Barrier(MPI_COMM_WORLD);        \
printf("Line: passed %d\n", __LINE__);     \
fflush(stdout);                     \
} while (0)

enum distribution {
    GAUSSIAN,
    EMPIRICAL,
    STATIC_VALUE
};
typedef enum distribution distribution_t;

enum halo {
    IMPORT,
    EXPORT
};

enum comm {
    MPIADVANCE,
    MPIS
};

typedef enum halo halo_t;
typedef enum comm comm_t;

static int nsamples = 25;
static int niterations = 1;

static std::string filepath = "";
static distribution_t distribution_type = EMPIRICAL;
static halo_t halo_type = EXPORT;
static comm_t comm_type = MPIADVANCE;
static std::string crs = "DEFAULT";

static int seed = -1;
static bool unique_seed = 0;
static int data_sent_max = -1;
static int nneighbors_max = -1;
struct Pattern {
    std::map < int, double > comm_partners; // normalized
    std::map < int, double > buffer_size; // fill-forward + normalized
    std::map < int, std::map < int, double >> dist_to_neighbors; // each inner map normalized
};

static std::map < std::string, Pattern > patterns;

// Random engine (seeded with current time)
std::mt19937 & global_rng() {
    static std::mt19937 rng(static_cast < unsigned long > (
        std::chrono::system_clock::now().time_since_epoch().count()));
    return rng;
}

// --- Sample from normalized map<int, double> ---
int sample_from_map(const std::map < int, double > & dist) {
    std::vector < int > keys;
    std::vector < double > weights;

    for (auto & [k, w]: dist) {
        keys.push_back(k);
        weights.push_back(w);
    }

    std::discrete_distribution < > d(weights.begin(), weights.end());
    return keys[d(global_rng())];
}

using json = nlohmann::json;

// Conversion from JSON → struct
void from_json(const json & j, Pattern & p) {
    // --- comm_partners ---
    {
        std::map < int, int > temp;
        for (auto & [k, v]: j.at("comm_partners").items()) {
     		temp[std::stoi(k) ] =v.get < int > ();
       //     temp[v.get < int > () ] = std::stoi(k);
        }

        // fill-forward
        int last_nonzero = 0;
        for (auto & [key, value]: temp) {
            if (value != 0) {
                last_nonzero = value;
            } else if (last_nonzero != 0) {
                value = last_nonzero;
            }
        }

        // normalize
        double total = 0.0;
        for (auto & [k, v]: temp) total += v;
        for (auto & [k, v]: temp) {
            p.comm_partners[k] = (total > 0) ? (double) v / total : 0.0;
        }

        nneighbors_max = std::max(p.comm_partners.rbegin() -> first, nneighbors_max);

    }

    // --- buffer_size ---
    {
        std::map < int, int > temp;
        for (auto & [k, v]: j.at("buffer_size").items()) {
            temp[std::stoi(k)] = v.get < int > ();
        }

        // fill-forward
        int last_nonzero = 0;
        for (auto & [key, value]: temp) {
            if (value != 0) {
                last_nonzero = value;
            } else if (last_nonzero != 0) {
                value = last_nonzero;
            }
        }

        // normalize
        double total = 0.0;
        for (auto & [k, v]: temp) total += v;
        for (auto & [k, v]: temp) {
            p.buffer_size[k] = (total > 0) ? (double) v / total : 0.0;
        }
        data_sent_max = std::max(p.buffer_size.rbegin() -> first, data_sent_max);

    }

    // --- dist_to_neighbors ---
    for (auto & [outer_k, inner_obj]: j.at("dist_to_neighbors").items()) {
        int outer_key = std::stoi(outer_k);
        std::map < int, int > temp;
        for (auto & [inner_k, inner_v]: inner_obj.items()) {
            temp[std::stoi(inner_k)] = inner_v.get < int > ();
        }

        double total = 0.0;
        for (auto & [k, v]: temp) total += v;

        std::map < int, double > norm_inner;
        for (auto & [k, v]: temp) {
            norm_inner[k] = (total > 0) ? (double) v / total : 0.0;
        }

        p.dist_to_neighbors[outer_key] = std::move(norm_inner);
    }
}
// Function to run a performance benchmark
// meat and potatos of the code
// copyed and changed form this code
// https://github.com/ECP-copa/Cabana/wiki/2-Programming-Guide
void run_benchmark() {

    auto TIME_START = std::chrono::high_resolution_clock::now();
    auto TIME_END = std::chrono::high_resolution_clock::now();

    std::chrono::duration < double, std::milli > duration = TIME_END - TIME_START;
    int comm_rank = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, & comm_rank);
    int comm_size = -1;
    MPI_Comm_size(MPI_COMM_WORLD, & comm_size);
    for (auto & [name, pattern]: patterns) {
        for (int sample_iter = 0; sample_iter < nsamples; sample_iter++) {
            std::list < int > neighbors_data;
            std::list < int > neighbors;
            std::set < int > seen_neighbors;
            int total_data = 0;
            int nneighborsV = -1;

            nneighborsV = sample_from_map(pattern.comm_partners);

            for (int i = 0; i < nneighborsV; ++i) {
                int data_sentV = sample_from_map(pattern.buffer_size);
                total_data += data_sentV;
                while (true) {

                    int distanceToN = sample_from_map(pattern.dist_to_neighbors[nneighborsV]);
                    int node = (distanceToN + comm_rank + comm_size) % comm_size;
                    if (seen_neighbors.find(node) == seen_neighbors.end()) {
                        seen_neighbors.insert(node);
                        neighbors.push_back(node);
                        neighbors_data.push_back(data_sentV / 8 / 2);
                        break;
                    }
                }
            }

            double haloTime;
            double resizeTime;
            double gatherTime;
            double apply;

            using DataTypes = Cabana::MemberTypes < double, double > ;
            const int VectorLength = 8;
            using MemorySpace = Kokkos::HostSpace;

            int num_tuple = data_sent_max;
            Cabana::AoSoA < DataTypes, MemorySpace, VectorLength > aosoa("my_aosoa",
                num_tuple);

            auto slice_ranks = Cabana::slice < 0 > (aosoa);
            auto slice_ids = Cabana::slice < 1 > (aosoa);
            for (int i = 0; i < num_tuple; ++i) {
                slice_ranks(i) = comm_rank;
                slice_ids(i) = i;
            }

            Kokkos::View < int * , MemorySpace > export_ranks("export_ranks",
                total_data);
            Kokkos::View < int * , MemorySpace > export_ids("export_ids", total_data);

            for (int i = 0; i < total_data; ++i) {
                export_ids(i) = -1;
                export_ranks(i) = -1;
            }

            int inum = 0;
            auto it_data = neighbors_data.begin();
            auto it_neighbors = neighbors.begin();

            while (it_data != neighbors_data.end() && it_neighbors != neighbors.end()) {

                for (int i = 0; i < * it_data; ++i) {
                    export_ids(inum) = i;
                    export_ranks(inum++) = *it_neighbors;
                }
                ++it_data;
                ++it_neighbors;
            }

            auto TIME_START_HALO = std::chrono::high_resolution_clock::now();

            MPI_Barrier(MPI_COMM_WORLD);
            if (comm_type == MPIADVANCE) {
                if (halo_type == EXPORT) {

                    TIME_START = std::chrono::high_resolution_clock::now();
                    Cabana::Halo < MemorySpace, Cabana::Export, Cabana::LocalityAware > halo(MPI_COMM_WORLD, num_tuple, export_ids, export_ranks);
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    haloTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();
                    aosoa.resize(halo.numLocal() + halo.numGhost());
                    fflush(stdout);
                    slice_ranks = Cabana::slice < 0 > (aosoa);
                    slice_ids = Cabana::slice < 1 > (aosoa);

                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    resizeTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();

                    auto gather = Cabana::createGather(halo, aosoa, 1.0);
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    gatherTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();

                    for (int i = 0; i < niterations; i++) {
                        gather.apply();
                    }
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    apply = duration.count();

                } else if (halo_type == IMPORT) {
                    TIME_START = std::chrono::high_resolution_clock::now();
                    Cabana::Halo < MemorySpace, Cabana::Import, Cabana::LocalityAware > halo(MPI_COMM_WORLD, num_tuple, export_ids, export_ranks);
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    haloTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();
                    aosoa.resize(halo.numLocal() + halo.numGhost());
                    slice_ranks = Cabana::slice < 0 > (aosoa);
                    slice_ids = Cabana::slice < 1 > (aosoa);

                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    resizeTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();

                    auto gather = Cabana::createGather(halo, aosoa, 1.0);

                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    gatherTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();

                    for (int i = 0; i < niterations; i++) {
                        gather.apply();
                    }
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    apply = duration.count();
                } else {
                    //error
                    Cabana::Halo < MemorySpace > halo(MPI_COMM_WORLD, num_tuple, export_ids, export_ranks);
                    aosoa.resize(halo.numLocal() + halo.numGhost());
                    slice_ranks = Cabana::slice < 0 > (aosoa);
                    slice_ids = Cabana::slice < 1 > (aosoa);
                    auto gather = Cabana::createGather(halo, aosoa, 1.0);

                    for (int i = 0; i < niterations; i++) {

                        gather.apply();
                    }
                }
            } else if (comm_type == MPIS) {
                if (halo_type == EXPORT) {

                    TIME_START = std::chrono::high_resolution_clock::now();
                    Cabana::Halo < MemorySpace, Cabana::Export, Cabana::Mpi > halo(MPI_COMM_WORLD, num_tuple, export_ids, export_ranks);
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    haloTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();
                    aosoa.resize(halo.numLocal() + halo.numGhost());
                    fflush(stdout);
                    slice_ranks = Cabana::slice < 0 > (aosoa);
                    slice_ids = Cabana::slice < 1 > (aosoa);

                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    resizeTime = duration.count();

      			    TIME_START = std::chrono::high_resolution_clock::now();

                    auto gather = Cabana::createGather(halo, aosoa, 1.0);
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    gatherTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();


                    for (int i = 0; i < niterations; i++) {
                        gather.apply();
                    }

                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    apply = duration.count();

                } else if (halo_type == IMPORT) {

                    TIME_START = std::chrono::high_resolution_clock::now();
                    Cabana::Halo < MemorySpace, Cabana::Import, Cabana::Mpi > halo(MPI_COMM_WORLD, num_tuple, export_ids, export_ranks);
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    haloTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();
                    aosoa.resize(halo.numLocal() + halo.numGhost());
                    slice_ranks = Cabana::slice < 0 > (aosoa);
                    slice_ids = Cabana::slice < 1 > (aosoa);

                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    resizeTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();

                    auto gather = Cabana::createGather(halo, aosoa, 1.0);
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    gatherTime = duration.count();

                    TIME_START = std::chrono::high_resolution_clock::now();

                    for (int i = 0; i < niterations; i++) {
                        gather.apply();
                    }
                    TIME_END = std::chrono::high_resolution_clock::now();
                    duration = TIME_END - TIME_START;
                    apply = duration.count();
                } else {
                    //error
                    Cabana::Halo < MemorySpace > halo(MPI_COMM_WORLD, num_tuple, export_ids, export_ranks);
                    aosoa.resize(halo.numLocal() + halo.numGhost());
                    slice_ranks = Cabana::slice < 0 > (aosoa);
                    slice_ids = Cabana::slice < 1 > (aosoa);
                    auto gather = Cabana::createGather(halo, aosoa, 1.0);

                    for (int i = 0; i < niterations; i++) {

                        gather.apply();
                    }
                }

            }
            auto TIME_END_Halo = std::chrono::high_resolution_clock::now();

            std::chrono::duration < double, std::milli > halo_gather_time = TIME_END_Halo - TIME_START_HALO;
            double halo_gather = halo_gather_time.count();

            #define DATA_SIZE 7
            double local_vals[DATA_SIZE] = {
                haloTime,
                resizeTime,
                gatherTime,
                apply,
                halo_gather,
                (double) nneighborsV,
                (double) inum
            };

            double min_vals[DATA_SIZE];
            double max_vals[DATA_SIZE];
            double sum_vals[DATA_SIZE];

            // Perform reductions
            MPI_Reduce(local_vals, min_vals, DATA_SIZE, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
            MPI_Reduce(local_vals, max_vals, DATA_SIZE, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
            MPI_Reduce(local_vals, sum_vals, DATA_SIZE, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

            if (comm_rank == 0) {

                const char * labels[DATA_SIZE] = {
                    "haloTime",
                    "resizeTime",
                    "gatherTime",
                    "apply",
                    "halo_gather_time",
                    "nneighbors",
                    "data_sent"
                };

                printf("%-20s %-12s %-12s %-12s\n", "Metric", "Min", "Max", "Average");
                printf("------------------------------------------------------------\n");

                for (int i = 0; i < DATA_SIZE; ++i) {
                    double avg = sum_vals[i] / comm_size;
                    printf("(%s)  %-20s %-.6f     %-.6f     %-.6f\n", name.c_str(), labels[i], min_vals[i], max_vals[i], avg);
                }
                printf("------------------------------------------------------------\n");
                fflush(stdout);
            }
        }
    }
}

// Function to print an error message and exit the program with an error code
void exitError(const std::string & error_message) {
    std::cerr << error_message << std::flush; // Use std::cerr for error messages

    std::exit(-1); // Exit the program with error code
}

// Function to set a value based on a command-line argument and check its validity.
void setAndCheckValue(int & value, TCLAP::ValueArg < int > & arg,
    const char * errorMessage, int minValue = 0, int maxValue = INT_MAX) {
    int tempValue = arg.getValue();
    // If the value from the argument is not -1 (indicating it was set by the user),
    // update the 'value' reference with the new value.
    if (tempValue != -1) {
        value = tempValue;
    }

    // Check if the value is within the allowed range (between minValue and maxValue).
    if (value < minValue || value > maxValue) {
        exitError(errorMessage);
    }
}

void parseArgs(int argc, char ** argv) {

    try {
        TCLAP::CmdLine cmd("\nNOTE: TODO",
            ' ', "1.0");

        TCLAP::ValueArg < std::string > filepathArg("f", "filepath", "Path to the BENCHMARK_CONFIG file", true, "NOFILE", "string");
        TCLAP::ValueArg < int > samplesArg("I", "samples", "Number of random samples to generate", false, 25, "int");
        TCLAP::ValueArg < int > iterationsArg("i", "iterations", "Number of updates each sample performs", false, niterations, "int");
        TCLAP::ValueArg < int > seedArg("S", "seed", "Positive integer to be used as seed for random number generation", false, -1, "int");
        TCLAP::SwitchArg useedArg("q", "unique-seed", "unique seed per rank", true);
        TCLAP::SwitchArg reportParamsArg("r", "report-params", "Enables parameter reporting for use with analysis scripts", false);
        TCLAP::ValueArg < std::string > distributionArg("d", "distribution", "Choose from: gaussian (default), empirical or static", false, "gaussian", "string");
        TCLAP::ValueArg < std::string > splitArg("s", "split-type", "Choose from: SOCKET|S|s (default), NUMA|U|u or NODE|N|n", false, "SOCKET", "string");
        TCLAP::ValueArg < std::string > commArg("c", "comm", "Choose from: MPIA|A|a (default) or MPI|M|m", false, "MPIA", "string");
        TCLAP::ValueArg < std::string > INorOUTArg("x", "type", "Choose from: EXPORT|E|e (default) or IMPORT|I|i", false, "EXPORT", "string");
        TCLAP::ValueArg < std::string > ALLTOALLV("a", "alltoallv", "Choose from: STANDARD|S|s (default) or LOCALITY|L|l", false, "STANDARD", "string");
        TCLAP::ValueArg < std::string > CRS(
            "C",
            "crs",
            "Choose CRS method: default| nonblocking | personalized | personalized_loc | nonblocking_loc |rma",
            false,
            "default",
            "string"
        );

        cmd.add(filepathArg);
        cmd.add(samplesArg);
        cmd.add(iterationsArg);
        cmd.add(seedArg);
        cmd.add(useedArg);
        cmd.add(distributionArg);
        cmd.add(reportParamsArg);
        cmd.add(commArg);
        cmd.add(INorOUTArg);
        cmd.add(ALLTOALLV);
        cmd.add(splitArg);
        cmd.add(CRS);

        cmd.parse(argc, argv);

        filepath = filepathArg.getValue();

        if (filepath != "NOFILE") {

            try {

                if (filepath.empty()) {
                    std::cerr << "Filepath is empty!" << std::endl;
                    return;
                }

                std::filesystem::path p(filepath);

                // Check if path exists and is a file
                if (std::filesystem::exists(p)) {

                    std::ifstream file(filepath);
                    if (!file.is_open()) {

                        exitError("Error: Could not open file. ");
                    }

                    json j;
                    file >> j;

                    for (auto & [pattern_name, pattern_json]: j.items()) {
                        patterns[pattern_name] = pattern_json.get < Pattern > ();
                    }

                } else {
                    exitError("The file does not exist.");
                }
            } catch (const std::exception & e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
        unique_seed = useedArg.getValue();

        std::string distribution = distributionArg.getValue();
        // For nsamples, no specific range, only non-negative check
        setAndCheckValue(nsamples, samplesArg, "ERROR: Invalid number of samples\n", 0);

        // For niterations, same non-negative check
        setAndCheckValue(niterations, iterationsArg, "ERROR: Invalid number of iterations\n", 0);

        if (distribution == "gaussian" ||
            distribution == "g") {
            distribution_type = GAUSSIAN;
        } else if (distribution == "empirical" ||
            distribution == "e") {
            distribution_type = EMPIRICAL;
        } else if (distribution == "static" ||
            distribution == "s") {
            distribution_type = STATIC_VALUE;
        } else {
            exitError("ERROR: Invalid distribution choice [empirical,gaussian]\n");
        }

        std::string comm = commArg.getValue();
        if (comm == "A" ||
            comm == "a" ||
            comm == "MPIA") {
            comm_type = MPIADVANCE;
            comm = "MPIADVANCE";
        } else if (comm == "M" ||
            comm == "m" ||
            comm == "MPI") {
            comm_type = MPIS;
            comm = "MPI";

        } else {
            exitError("ERROR: Invalid Backend choice [MPIA,MPIA]\n");
        }


        std::string split = splitArg.getValue();
        if (split == "U" ||
            split == "u" ||
            split == "NUMA") {
            MPIL_Set_split(NUMA);
            split = "NUMA";
        } else if (split == "N" ||
                split == "n" ||
                split == "NODE") {


            split = "NODE";
            MPIL_Set_split(NUMA);

        }  else if (split == "S" ||
                        split == "s" ||
                        split == "SOCKET") {

            split = "SOCKET";
            MPIL_Set_split(SOCKET);
        }else {
                    exitError("ERROR: Invalid split choice [SOCKET,NODE,NUMA]\n");
        }




        std::string discovery = CRS.getValue();
        if (discovery == "rma") {
            MPIL_Set_alltoall_crs(ALLTOALL_CRS_RMA);

        } else
        if (discovery == "default") {

        } else
        if (discovery == "nonblocking") {
            MPIL_Set_alltoall_crs(ALLTOALL_CRS_NONBLOCKING);

        } else if (discovery == "personalized") {

            MPIL_Set_alltoall_crs(ALLTOALL_CRS_PERSONALIZED);

        } else if (discovery == "personalized_loc") {

            MPIL_Set_alltoall_crs(ALLTOALL_CRS_PERSONALIZED_LOC);
        } else if (discovery == "nonblocking_loc") {

            MPIL_Set_alltoall_crs(ALLTOALL_CRS_NONBLOCKING_LOC);
        } else {

            exitError("ERROR: Invalid pattern choice [default,nonblocking, personalized, personalized_loc, nonblocking_loc]\n");
        }

        crs = discovery;

        std::string type = INorOUTArg.getValue();
        if (type == "E" ||
            type == "e" ||
            type == "EXPORT") {
            halo_type = EXPORT;
            type = "EXPORT";
        } else if (type == "I" ||
            type == "i" ||
            type == "IMPORT") {
            halo_type = IMPORT;
            type = "IMPORT";
        } else {
            exitError("ERROR: Invalid Patern choice [EXPORT,IMPORT]\n");
        }

        std::string alltoallv = ALLTOALLV.getValue();
        if (alltoallv == "S" ||
            alltoallv == "s" ||
            alltoallv == "STANDARD") {
            mpil_neighbor_alltoallv_init_implementation = NEIGHBOR_ALLTOALLV_INIT_STANDARD;
            alltoallv = "NEIGHBOR_ALLTOALLV_INIT_STANDARD";
        } else if (alltoallv == "L" ||
            alltoallv == "l" ||
            alltoallv == "LOCALITY") {
            mpil_neighbor_alltoallv_init_implementation = NEIGHBOR_ALLTOALLV_INIT_LOCALITY;
            alltoallv = "NEIGHBOR_ALLTOALLV_INIT_LOCALITY";
        } else {
            exitError("ERROR: Invalid alltoallv choice [LOCALITY,STANDARD]\n");
        }

        int seedholder = seedArg.getValue();
        if (seed != -1 && seedholder == -1) {
            seed = time(NULL);
        }
        int comm_rank = -1;
        MPI_Comm_rank(MPI_COMM_WORLD, & comm_rank);

        if (unique_seed) {

            srand(seed + comm_rank);
        } else {
            srand(seed);
        }

        if (comm_rank == 0) {
            if (reportParamsArg.getValue()) {
                printf("------------------------------------------------------------\n");
                printf("-MPI: %s\n", comm.c_str());
                printf("-halotype: %s\n", type.c_str());

                printf("-File: %s\n", filepath.c_str());
                printf("-samples: %i\n", nsamples);
                printf("-iterations: %i\n", niterations);
                printf("-CRS: %s\n", crs.c_str());
                printf("-alltoallv: %s\n", alltoallv.c_str());
                printf("-split: %s\n", split.c_str());
                printf("------------------------------------------------------------\n");
            } else {
                printf("------------------------------------------------------------\n");
            }
        }

    } catch (TCLAP::ArgException & e) {
        std::cerr << "Error: " << e.error() << " for argument " << e.argId() << std::endl;
        exit(-1);
    }
}

int main(int argc, char ** argv) {

    MPI_Init( & argc, & argv);

    {
        // Parse command-line arguments to set global static variables
        parseArgs(argc, argv);

        Kokkos::ScopeGuard scope_guard(argc, argv);
        // Run the benchmark
        run_benchmark();
    }

    MPI_Finalize();

    return 0;
}
