#include <iostream>

#include <memory>

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





using json = nlohmann::json;

struct Bin {
    double bin_min;
    double bin_max;
    double bin_prop;
    double bin_mean;
    double bin_stdev;
};


enum distribution
{
	GAUSSIAN,
	EMPIRICAL,
	STATIC_VALUE
};

typedef enum distribution distribution_t;

enum prefix
{
	A,
	B,
	K,
	M,
	G
};

typedef enum prefix prefix_t;

static int typesize = 8;
static int numpes = 0;
static int nsamples = 25;
static int niterations = 100;


static int nneighbors = -1;
static int nneighbors_stdv = -1;
static std::vector<Bin> nneighbors_bins;


static int nowned = -1;
static int nowned_stdv = -1;
static std::vector<Bin> nowned_bins;

static int nremote = -1;
static int nremote_stdv = -1;
static std::vector<Bin> nremote_bins;

static int blocksz = -1;
static int blocksz_stdv = -1;
static std::vector<Bin> blocksz_bins;

static int stride = -1;
static int stride_stdv = -1;
static std::vector<Bin> stride_bins;



static int unit_div = 1;
static prefix unit_symbol = A;
static std::string filepath = "";
static distribution_t distribution_type = GAUSSIAN;

// static bool irregularity = 1;
// static bool irregularity_owned = 1;
// static bool irregularity_neighbors = 1;
// static bool irregularity_stride = 1;
// static bool irregularity_blocksz = 1;
// static bool irregularity_remote = 1;
static bool report_params = 0;
static int seed = -1;
static bool unique_seed = 0;
static int neighbor_discovery_algo = 0;




int gauss_dist(double mean, double stdev)
{
  	
	// Generates a Gaussian (normal) distribution with only positive values.
	// generates two random numbers that form the seeds
	// of the transform
	double u1, u2, r, theta;
	int generated = -1;

	while (generated <= 0)
	{
		u1 = (double)rand() / RAND_MAX;
		u2 = (double)rand() / RAND_MAX;

		// generates the R and Theta values from the above
		// documentation
		r = sqrt(-2. * log(u1));
		theta = (2 * M_PI * u2);

		// an additional number can be generated in the
		// same distribution using the alternate form
		// ((r*sin(theta)) * stdev) + mean

		generated = round(((r * cos(theta)) * stdev) + mean);
	}

	return generated;
}


// Function to calculate an empirical distribution value based on Bin objects.
int empirical_dist(std::vector<Bin> bins)
{
  	
	double rand = static_cast<double>(std::rand()) / RAND_MAX;
	double prob = 0.0;

    // Iterate over all bins except the last one.
	for (int i = 0; i < bins.size()-1; ++i) {
		Bin& bin = bins[i];  
		 // Accumulate the probability
		prob+=bin.bin_prop;


		// Generate  Gaussian distribution for the bin.
        // Repeat until the value is within  minimum and maximum limits.
		if (prob<= rand)
		{
			double value_at_bin=-1;
			do {
				value_at_bin= gauss_dist(bin.bin_mean ,bin.bin_stdev);
			} while (value_at_bin<= bin.bin_min || value_at_bin >=bin.bin_max);
			return value_at_bin;
	
		}
	}
	// If the loop has finished, it means the random number corresponds to the last bin.
	Bin& lastBin = bins.back();
	double value_at_bin=-1;
	do {
		value_at_bin= gauss_dist(lastBin.bin_mean ,lastBin.bin_stdev);
	} while (value_at_bin<= lastBin.bin_min || value_at_bin >= lastBin.bin_max);
	// Return the generated value from the last bin.	
	return value_at_bin;
}

// Function to run a performance benchmark
// meat and potatos of the code
// copyed and changed form this code
// https://github.com/ECP-copa/Cabana/wiki/2-Programming-Guide
void run_benchmark()
{
	
	int nowned_orig = nowned;
	int nneighbors_orig = nneighbors;
	int nremote_orig = nremote;
	int blocksz_orig = blocksz;
	int stride_orig = stride;

	
	int comm_rank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
	int comm_size = -1;
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
Kokkos::Profiling::pushRegion("className::functionName");


    Kokkos::Profiling::pushRegion("Bench_mark_loop");
	for (int sample_iter = 0; sample_iter < nsamples ; sample_iter++)
	{



		Kokkos::Profiling::pushRegion("set_distribution");
        // Modify parameters based on the chosen distribution type
		if (distribution_type == GAUSSIAN)
		{
			nowned = gauss_dist(nowned_orig, nowned_stdv);
			nremote = gauss_dist(nremote_orig, nremote_stdv);
			blocksz = gauss_dist(blocksz_orig, blocksz_stdv);
			nneighbors = gauss_dist(nneighbors_orig, nneighbors_stdv);
			stride = gauss_dist(stride_orig, stride_stdv);
			
		}else if (distribution_type == EMPIRICAL){


			nowned     = empirical_dist(nowned_bins);
			nremote    = empirical_dist(nremote_bins);
			blocksz    = empirical_dist(blocksz_bins);
			nneighbors = empirical_dist(nneighbors_bins);
			stride     = empirical_dist(stride_bins);

		}else if (distribution_type== STATIC_VALUE)
			nowned     = nowned_orig;
			nremote    = nneighbors_orig;
			blocksz    = nremote_orig;
			nneighbors = blocksz_orig;
			stride     = stride_orig;

		}

		Kokkos::Profiling::popRegion();

	
        // Debug output if needed (currently disabled)
		if (0)
		{
			
			printf("PARAM: nowned - %d\n", nowned);
			printf("PARAM: nremote - %d\n", nremote);
			printf("PARAM: blocksize - %d\n", blocksz);
			printf("PARAM: stride - %d\n", stride);
			printf("PARAM: nneighbors - %d\n", nneighbors);
		}
        // Run the benchmark using a specific neighbor discovery algorithm
        // 0 is bulit into Cabana
		if (neighbor_discovery_algo == 0)
		{

			using DataTypes = Cabana::MemberTypes<int, int>;
			const int VectorLength = 8;
			using MemorySpace = Kokkos::HostSpace;

			int num_tuple = nowned;
			Cabana::AoSoA<DataTypes, MemorySpace, VectorLength> aosoa("A", num_tuple);
			Kokkos::Profiling::pushRegion("fill_arrays");
			auto slice_ranks = Cabana::slice<0>(aosoa);
			auto slice_ids = Cabana::slice<1>(aosoa);
			for (int i = 0; i < num_tuple; ++i)
			{
				slice_ranks(i) = comm_rank;
				slice_ids(i) = i + (num_tuple * comm_rank);
			}
			Kokkos::Profiling::popRegion();


			Kokkos::Profiling::pushRegion("fill_export_ranks");
			Kokkos::View<int *, MemorySpace> export_ranks("export_ranks", num_tuple);
			int remainder = nneighbors % 2;
			int offset = 0;
			int *preneighbors = new int[nneighbors + 1];
			int *recvbuf = new int[(nneighbors + 1) * comm_size];
			for (int i = -nneighbors / 2; i <= (nneighbors / 2) + remainder; i++)
			{
				int partner = (comm_size + i + comm_rank) % comm_size;
				preneighbors[offset] = partner;
				offset++;
			}
			std::vector<int> neighbors;

			int curneighbor = 0;
			int inum = 0;
			for (int i = 0; i < num_tuple; ++i)
			{
				export_ranks(i) = -1;
			}

			for (int j = 0; j < num_tuple / (stride + blocksz); j++)
			{

				for (int i = 0; i < (stride + blocksz); i++)
				{
					if (i < stride)
					{
						export_ranks(inum++) = preneighbors[curneighbor];
					}
					else
					{
						inum++;
					}

					if (inum % (num_tuple / (nneighbors + 1)) == 0)
					{
						curneighbor++;
					}
				}
			}
			Kokkos::Profiling::popRegion();

			Cabana::Distributor<MemorySpace> distributor(MPI_COMM_WORLD, export_ranks);
			//runs this distributor niterations amount of times  ^^^^
			Kokkos::Profiling::pushRegion("iterations");
			for (int i = 0; i < niterations ; i++)
			{

					Cabana::AoSoA<DataTypes, MemorySpace, VectorLength> destination(
					"destination", distributor.totalNumImport());
				Cabana::migrate(distributor, aosoa, destination);
				auto slice_ranks_dst = Cabana::slice<0>(destination);
				auto slice_ids_dst = Cabana::slice<1>(destination);
				Cabana::migrate(distributor, slice_ranks, slice_ranks_dst);
				Cabana::migrate(distributor, slice_ids, slice_ids_dst);
				Cabana::migrate(distributor, aosoa);
				slice_ranks = Cabana::slice<0>(aosoa);
				slice_ids = Cabana::slice<1>(aosoa);

			}
			Kokkos::Profiling::popRegion();

	



	}

	Kokkos::Profiling::popRegion();

}




// Function to parse a JSON configuration file
void parse_config_file(std::string config_file)
{
	std::ifstream file(config_file);

	if (!file.is_open())
	{
		std::cerr << "Error: Could not open file!" << std::endl;
	}
	else
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string input = buffer.str();
		nlohmann::json j = nlohmann::json::parse(input);
		//     Accessing the data
		for (const auto &param : j["parameters"])
		{

			std::string name = param["name"].get<std::string>();

			int mean = param["mean"].get<int>();

			int stddev = param["stdev"].get<int>();  
			std::vector<Bin> bins;
			for (const auto& bin_json : param["bins"]) {
					Bin bin;
					bin.bin_min = bin_json["bin_min"];
					bin.bin_max = bin_json["bin_max"];
					bin.bin_prop = bin_json["bin_prop"];
					bin.bin_mean = bin_json["bin_mean"];
					bin.bin_stdev = bin_json["bin_stdev"];
					bins.push_back(bin);
			}

		
			if (name == "nowned")
			{
				nowned = mean;
				nowned_stdv = stddev;
				nowned_bins = bins;


			}
			else if (name == "nremote")
			{
				nremote = mean;
				nremote_stdv = stddev;
				nremote_bins = bins;


				
			}
			else if (name == "blocksize")
			{
				blocksz = mean;
				blocksz_stdv = stddev;
				blocksz_bins = bins;
			}
			else if (name == "comm_partners")
			{
				nneighbors = mean;
				nneighbors_stdv = stddev;
				nneighbors_bins = bins;
			}
			else if (name == "stride")
			{
				stride = mean;
				stride_stdv = stddev;
				stride_bins = bins;
			}
			else
			{
			    // Handle any other parameters
                // For now, it just has a placeholder comment for future functionality
                // mostlike an error
			}
		}
	}
}


// Function to print an error message and exit the program with an error code
void exitError(const std::string &error_message)
{
	std::cerr << error_message << std::flush; // Use std::cerr for error messages

	std::exit(-1); // Exit the program with error code
}




// Function to set a value based on a command-line argument and check its validity.
void setAndCheckValue(int &value, TCLAP::ValueArg<int> &arg,
					  const char *errorMessage, int minValue = 0, int maxValue = INT_MAX)
{
	int tempValue = arg.getValue();
 	// If the value from the argument is not -1 (indicating it was set by the user),
    // update the 'value' reference with the new value.
	if (tempValue != -1)
	{
		value = tempValue;
	}

	// Check if the value is within the allowed range (between minValue and maxValue).
	if (value < minValue || value > maxValue)
	{
		exitError(errorMessage);
	}
}




void parseArgs(int argc, char **argv)
{

	try
	{
		TCLAP::CmdLine cmd("\nNOTE: Setting parameters for the benchmark such as (neighbors, owned, remote, blocksize, and stride)"
						   "sets parameters to those values for the reference benchmark."
						   "Those parameters are then randomized for the irregular samples"
						   "where the user-set parameters become averages for the random generation."
						   "Use the `--disable-irregularity` flag to only run the reference benchmark.",
						   ' ', "1.0");

		TCLAP::ValueArg<std::string> filepathArg("f", "filepath", "Path to the BENCHMARK_CONFIG file", false, "NOFILE", "string");
		TCLAP::ValueArg<int> typeSizeArg("t", "typesize", "Size of the variable being sent (in bytes)", false, 8, "int");
		TCLAP::ValueArg<int> samplesArg("I", "samples", "Number of random samples to generate", false, 25, "int");
		TCLAP::ValueArg<int> iterationsArg("i", "iterations", "Number of updates each sample performs", false, 100, "int");
		TCLAP::ValueArg<int> neighborsArg("n", "neighbors", "Average number of neighbors each process communicates with", false, -1, "int");
		TCLAP::ValueArg<int> neighborsStdvArg("N", "neighbors_stdv", "Standard deviation of the number of neighbors each process communicates with", false, -1, "int");
		TCLAP::ValueArg<int> ownedAvgArg("o", "owned_avg", "Average byte count for data owned per node", false, -1, "int");
		TCLAP::ValueArg<int> ownedStdvArg("O", "owned_stdv", "Standard deviation byte count for data owned per node", false, -1, "int");
		TCLAP::ValueArg<int> remoteAvgArg("r", "remote_avg", "Average amount of data each process receives", false, -1, "int");
		TCLAP::ValueArg<int> remoteStdvArg("R", "remote_stdv", "Standard deviation of the amount of data each process receives", false, -1, "int");
		TCLAP::ValueArg<int> blockSizeAvgArg("b", "blocksize_avg", "Average size of transmitted blocks", false, -1, "int");
		TCLAP::ValueArg<int> blockSizeStdvArg("B", "blocksize_stdv", "Standard deviation of transmitted block sizes", false, -1, "int");
		TCLAP::ValueArg<int> strideArg("s", "stride", "Average size of stride", false, -1, "int");
		TCLAP::ValueArg<int> strideStdvArg("T", "stride_stdv", "Standard deviation of stride", false, -1, "int");
		TCLAP::ValueArg<int> seedArg("S", "seed", "Positive integer to be used as seed for random number generation", false, -1, "int");
		TCLAP::ValueArg<int> neighbordiscoverArg("n", "neighbor algo", "0 Default built-in discovery in cabana", false, 0, "int");
		TCLAP::ValueArg<std::string> distributionArg("d", "distribution", "Choose from: gaussian (default), empirical", false, "gaussian", "string");
		TCLAP::ValueArg<std::string> unitsArg("u", "units", "Choose from: a,b,k,m,g (auto, bytes, kilobytes, etc.)", false, "auto", "string");
		TCLAP::SwitchArg useedArg("q", "unique-seed", "unique seed per rank", false);
		TCLAP::SwitchArg reportParamsArg("", "report-params", "Enables parameter reporting for use with analysis scripts", false);
		TCLAP::SwitchArg disableirregularityArg("", "disable-irregularity", "Use the `--disable-irregularity` flag to only run the reference benchmark.", false);
		cmd.add(filepathArg);
		cmd.add(typeSizeArg);
		cmd.add(samplesArg);
		cmd.add(iterationsArg);
		cmd.add(neighborsArg);
		cmd.add(neighborsStdvArg);
		cmd.add(ownedAvgArg);
		cmd.add(ownedStdvArg);
		cmd.add(remoteAvgArg);
		cmd.add(remoteStdvArg);
		cmd.add(blockSizeAvgArg);
		cmd.add(blockSizeStdvArg);
		cmd.add(strideArg);
		cmd.add(strideStdvArg);
		cmd.add(seedArg);
		cmd.add(distributionArg);
		cmd.add(unitsArg);
		cmd.add(reportParamsArg);
		cmd.add(neighbordiscoverArg);
		cmd.add(disableirregularityArg);
		cmd.add(useedArg);
		cmd.parse(argc, argv);

		filepath = filepathArg.getValue();
		bool config_file_used = false; // todo

		if (filepath != "NOFILE")
		{

			try
			{
				if (filepath.empty())
				{
					std::cerr << "Filepath is empty!" << std::endl;
					return;
				}

				std::filesystem::path p(filepath);

				// Check if path exists and is a file
				if (std::filesystem::exists(p))
				{
					parse_config_file(filepath);
				}
				else
				{
					exitError("The file does not exist.");
				}
			}
			catch (const std::exception &e)
			{
				std::cerr << "Error: " << e.what() << std::endl;
			}
		}
		unique_seed = useedArg.getValue();
		neighbor_discovery_algo = neighbordiscoverArg.getValue();

		setAndCheckValue(typesize, typeSizeArg, "ERROR: Invalid typesize\n", 1, 8);

		// For nsamples, no specific range, only non-negative check
		setAndCheckValue(nsamples, samplesArg, "ERROR: Invalid number of samples\n", 0);

		// For niterations, same non-negative check
		setAndCheckValue(niterations, iterationsArg, "ERROR: Invalid number of iterations\n", 0);

		// For nneighbors, same non-negative check
		setAndCheckValue(nneighbors, neighborsArg, "ERROR: Invalid number of neighbors\n", 0);

		// For nneighbors_stdv, no specific range, only non-negative check
		setAndCheckValue(nneighbors_stdv, neighborsStdvArg, "ERROR: Invalid neighbors std\n", 0);

		// For nowned, no specific range, only non-negative check
		setAndCheckValue(nowned, ownedAvgArg, "ERROR: Invalid number of owned\n", 0);

		// For nowned_stdv, no specific range, only non-negative check
		setAndCheckValue(nowned_stdv, ownedStdvArg, "ERROR: Invalid owned std\n", 0);

		// For nremote, no specific range, only non-negative check
		setAndCheckValue(nremote, remoteAvgArg, "ERROR: Invalid number of remote\n", 0);

		// For nremote_stdv, no specific range, only non-negative check
		setAndCheckValue(nremote_stdv, remoteStdvArg, "ERROR: Invalid remote std\n", 0);

		// For blocksz, no specific range, only non-negative check
		setAndCheckValue(blocksz, blockSizeAvgArg, "ERROR: Invalid block size\n", 0);

		// For blocksz_stdv, no specific range, only non-negative check
		setAndCheckValue(blocksz_stdv, blockSizeStdvArg, "ERROR: Invalid block size std\n", 0);

		// For stride, no specific range, only non-negative check
		setAndCheckValue(stride, strideArg, "ERROR: Invalid stride\n", 0);

		// For stride_stdv, no specific range, only non-negative check
		setAndCheckValue(stride_stdv, strideStdvArg, "ERROR: Invalid stride std\n", 0);

		std::string unit = unitsArg.getValue();

		std::unordered_map<std::string, std::pair<char, int>> unit_map = {
			{"auto",
			 {A,
			  1}},
			{"a",
			 {A,
			  1}},
			{"bytes",
			 {A,
			  1}},
			{"b",
			 {A,
			  1}},
			{"kilobytes",
			 {K,
			  1024}},
			{"k",
			 {K,
			  1024}},
			{"megabytes",
			 {M,
			  1024 * 1024}},
			{"m",
			 {M,
			  1024 * 1024}},
			{"gigabytes",
			 {G,
			  1024 * 1024 * 1024}},
			{"g",
			 {G,
			  1024 * 1024 * 1024}}};

		auto it = unit_map.find(unit);
		if (it != unit_map.end())
		{
			char unit_symbol = it->second.first;
			int unit_div = it->second.second;

			std::cout << "Unit Symbol: " << unit_symbol << std::endl;
			std::cout << "Unit Division: " << unit_div << std::endl;
		}
		else
		{
			exitError("ERROR: Invalid formatting choice [b, k, m, g]");
		}

		std::string distribution = distributionArg.getValue();

		if (distribution == "gaussian" ||
			distribution == "g")
		{
			distribution_type = GAUSSIAN;
		}
		else if (distribution == "empirical" ||
				 distribution == "e")
		{
			distribution_type = EMPIRICAL;
		}
		 else if (distribution == "static" ||
				 distribution == "s")
		{
			distribution_type = STATIC_VALUE;
		}else
		{
			exitError("ERROR: Invalid distribution choice [empirical,gaussian]\n");
		}

		int seedholder = seedArg.getValue();
		if (seed != -1 && seedholder == -1)
		{
			seed = time(NULL);
		}

		if (unique_seed)
		{
			int comm_rank = -1;
			MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
			srand(seed + comm_rank);
		}
		else
		{
			srand(seed);
		}

//		irregularity = disableirregularityArg.getValue();


	}
	catch (TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " for argument " << e.argId() << std::endl;
		exit(-1);
	}
}

int main(int argc, char **argv)
{

	MPI_Init(&argc, &argv);
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