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


static std::map<int, std::map<int, double>> distToNeighbors;


static double nneighbors = -1;//comm_partners
static double nneighbors_stdv = -1;
static std::vector<Bin> nneighbors_bins;
static int nneighbors_min = -1;
static int nneighbors_max = -1;

static double data_sent = -1;
static double data_sent_stdv = -1;
static std::vector<Bin> data_sent_bins;
static int data_sent_min = -1;
static int data_sent_max = -1;



static double delay = -1;
static double delay_stdv = -1;
static std::vector<Bin> delay_bins;
static int delay_min = -1;
static int delay_max = -1;



static int unit_div = 1;
static prefix unit_symbol = A;
static std::string filepath = "";
static distribution_t distribution_type = GAUSSIAN;

static bool report_params = 0;
static int seed = -1;
static bool unique_seed = 0;
static int neighbor_discovery_algo = 0;

int getDistToNeighbors(int nneighbors){
	int sample = nneighbors;

	if (distToNeighbors.find(nneighbors) == distToNeighbors.end()) {
		auto it = distToNeighbors.upper_bound(nneighbors);
		sample = it->first;
	}
//	distToNeighbors[ sample ];
	printf("getDistToNeighbors: nneighbors: %d\n", nneighbors);
	double threshold = static_cast<double>(std::rand()) / RAND_MAX;
	double sum = 0.0;
	for (const auto& [innerKey, weight] : distToNeighbors[sample]) {
		sum += weight;
		if (sum >= threshold) {
			return innerKey;
			break;
		}
	}
}



int gauss_dist(double mean, double stdev)
{

	// Generates a Gaussian (normal) distribution with only positive values.
	// generates two random numbers that form the seeds
	// of the transform
	double u1, u2, r, theta;
	int generated = -1;

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


	return generated;
}

int gauss_dist(double mean, double stdev,double min,double max)
{

	int generated=-1;
	do {
		generated= gauss_dist(mean , stdev);

	} while (generated<= min || generated >=max);
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
		if (prob >= rand)
		{
			int random = gauss_dist(bin.bin_mean ,bin.bin_stdev,bin.bin_min,bin.bin_max);
			return random;

		}
	}
	// If the loop has finished, it means the random number corresponds to the last bin.
	Bin& lastBin = bins.back();
	return gauss_dist(lastBin.bin_mean ,lastBin.bin_stdev,lastBin.bin_min,lastBin.bin_max);
}

// Function to run a performance benchmark
// meat and potatos of the code
// copyed and changed form this code
// https://github.com/ECP-copa/Cabana/wiki/2-Programming-Guide
void run_benchmark()
{
	printf("-bencchmark-\n");

	double nneighbors_orig        = nneighbors;
	double data_sent_orig         = data_sent;


	int comm_rank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
	int comm_size = -1;
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);


	auto TIME_START = std::chrono::high_resolution_clock::now();
	auto TIME_END = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = TIME_END - TIME_START;

	for (int sample_iter = 0; sample_iter < nsamples ; sample_iter++)
	{


        std::vector<double> time;

        // Modify parameters based on the chosen distribution type
		std::list<int> neighbors_data;
		std::list<int> neighbors;
		std::set<int> seen_neighbors;

		int total_data=0;
		TIME_START = std::chrono::high_resolution_clock::now();

		if (distribution_type == GAUSSIAN)
		{
			nneighbors = gauss_dist(nneighbors_orig, nneighbors_stdv,nneighbors_min,nneighbors_max);
			printf("-gauss_dist nneighbors - %i \n",nneighbors);

			for (int i = 0; i < nneighbors; ++i){
				data_sent = gauss_dist(data_sent_orig, data_sent_stdv,data_sent_min,data_sent_max);
				total_data+=data_sent;
				while (true) {
					printf("-bencchmark while \n");

					int distanceToN = getDistToNeighbors(nneighbors);
					printf("-bencchmark while - %i \n",distanceToN);

					// todo int distanceToN = gauss_dist(dist_to_neighbors_orig, dist_to_neighbors_stdv,dist_to_neighbors_min,dist_to_neighbors_max);
					if (distanceToN!=0&&seen_neighbors.find(distanceToN) == seen_neighbors.end()) {
						seen_neighbors.insert(distanceToN);
						neighbors.push_back(distanceToN);
						neighbors_data.push_back(data_sent);
						break;
					}
				}
			}
		}else if (distribution_type == EMPIRICAL){

			nneighbors     = empirical_dist(nneighbors_bins);
			for (int i = 0; i < nneighbors; ++i){

				data_sent = empirical_dist(data_sent_bins);

				total_data+=data_sent;
				while (true) {

					int distanceToN = getDistToNeighbors(nneighbors);
					if (distanceToN!=0&& seen_neighbors.find(distanceToN) == seen_neighbors.end()) {
						seen_neighbors.insert(distanceToN);
						neighbors.push_back(distanceToN);
						neighbors_data.push_back(data_sent);
						break;
					}
				}
			}
		}
		printf("-bencchmark after \n");

		TIME_END = std::chrono::high_resolution_clock::now();
		duration = TIME_END - TIME_START;
		MPI_Barrier(MPI_COMM_WORLD);

		double distribution_time = duration.count() * 1e6;

		TIME_START = std::chrono::high_resolution_clock::now();
		using DataTypes = Cabana::MemberTypes<int, int>;
		const int VectorLength = 8;
		using MemorySpace = Kokkos::HostSpace;

		int num_tuple = total_data;//todo

		Cabana::AoSoA<DataTypes, MemorySpace, VectorLength> aosoa("A", num_tuple);
		auto slice_ranks = Cabana::slice<0>(aosoa);
		auto slice_ids = Cabana::slice<1>(aosoa);

		Kokkos::View<int *, MemorySpace> export_ranks("export_ranks", num_tuple);

		for (int i = 0; i < num_tuple; ++i)
		{
			slice_ranks(i) = comm_rank;
			slice_ids(i) = i + (num_tuple * comm_rank);
			export_ranks(i)= -1;
		}




		auto it_data = neighbors_data.begin();
		auto it_neighbors = neighbors.begin();
		int inum =0;
		auto Tfillranks = std::chrono::high_resolution_clock::now();

		while (it_data != neighbors_data.end() && it_neighbors != neighbors.end()) {

			for (int i = 0; i < *it_data; ++i){
			export_ranks(inum++) = (*it_neighbors+comm_rank+comm_size)%comm_size;
			}
			++it_data;
			++it_neighbors;
		}

		TIME_END = std::chrono::high_resolution_clock::now();
		duration = TIME_END - TIME_START;
		double fill_space_time = duration.count() * 1e6;

		TIME_START = std::chrono::high_resolution_clock::now();

		Cabana::Distributor<MemorySpace> distributor(MPI_COMM_WORLD, export_ranks);


//import halo
// https://github.com/CUP-ECS/Cabana/blob/add-MPI_Advance/core/src/Cabana_Halo.hpp --constructor
//
////spack develop cabana@master
//

		TIME_END = std::chrono::high_resolution_clock::now();
		duration = TIME_END - TIME_START;
		double distributor_time = duration.count() * 1e6;

		TIME_START = std::chrono::high_resolution_clock::now();
//		for (int i = 0; i < niterations ; i++)

		for (int i = 0; i < 1 ; i++)
		{
			//runs this distributor niterations amount of times  ^^^^
			Cabana::AoSoA<DataTypes, MemorySpace, VectorLength> destination(
				"destination", distributor.totalNumImport());

			Cabana::migrate(distributor, aosoa, destination);
			auto slice_ranks_dst = Cabana::slice<0>(destination);
			auto slice_ids_dst = Cabana::slice<1>(destination);
		}

		TIME_END = std::chrono::high_resolution_clock::now();
		duration = TIME_END - TIME_START;
		double iterations_time = duration.count() * 1e6;


		int data_size =6;

		double local_vals[data_size] = {
			iterations_time,
			distributor_time,
			fill_space_time,
			distribution_time,
			nneighbors,
			inum
		};

		double min_vals[data_size];
		double max_vals[data_size];
		double sum_vals[data_size];

		// Perform reductions
		MPI_Reduce(local_vals, min_vals, data_size, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
		MPI_Reduce(local_vals, max_vals, data_size, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
		MPI_Reduce(local_vals, sum_vals, data_size, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

		if(comm_rank ==0){
			const char* labels[data_size] = {
				"iterations_time",
				"distributor_time",
				"fill_space_time",
				"distribution_time",
				"neighbors",
				"data sent"
			};

			printf("%-20s %-12s %-12s %-12s\n", "Metric", "Min", "Max", "Average");
			printf("------------------------------------------------------------\n");

			for (int i = 0; i < data_size; ++i) {
				double avg = sum_vals[i] / comm_size;
				printf("%-20s %-.6f     %-.6f     %-.6f\n", labels[i], min_vals[i], max_vals[i], avg);
			}
			printf("------------------------------------------------------------\n");
			fflush(stdout);
		}



	}








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

			if (name == "dist_to_neighbors")
			{


				for (auto& outer_pair : param["bins"].items()) {
					int outer_key = std::stoi(outer_pair.key()); // Convert outer key to int
					const json& inner_obj = outer_pair.value();  // The inner JSON object

					std::map<int, double> inner_map;

					for (auto& inner_pair : inner_obj.items()) {
						int inner_key = std::stoi(inner_pair.key());       // Convert inner key
						double inner_value = inner_pair.value().get<double>(); // Get value as double
						inner_map[inner_key] = inner_value;                // Store in map
					}

					distToNeighbors[outer_key] = inner_map;
				}
			}else{
				double mean = param["mean"].get<double>();

				double stddev = param["stdev"].get<double>();


				int min = static_cast<int>(std::round(param["min"].get<double>()));
				int max = static_cast<int>(std::round(param["max"].get<double>()));

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

				std::sort(bins.begin(), bins.end(), [](const Bin& a, const Bin& b) {
					return a.bin_prop > b.bin_prop;
				});

				if (name == "comm_partners")
				{
					nneighbors = mean;
					nneighbors_stdv = stddev;
					nneighbors_min = min;
					nneighbors_max = max;
					nneighbors_bins = bins;
				}

				else if (name == "delay")
				{
					delay = mean;
					delay_stdv = stddev;
					delay_min = min;
					delay_max = max;

					delay_bins = bins;
				}else if (name == "data_sent")
				{
					data_sent = mean;
					data_sent_stdv = stddev;
					data_sent_min = min;
					data_sent_max = max;

					data_sent_bins = bins;
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
		TCLAP::CmdLine cmd("\nNOTE: TODO",
						   ' ', "1.0");

		TCLAP::ValueArg<std::string> filepathArg("f", "filepath", "Path to the BENCHMARK_CONFIG file", false, "NOFILE", "string");

		TCLAP::ValueArg<int> samplesArg("I", "samples", "Number of random samples to generate", false, 25, "int");

		TCLAP::ValueArg<int> iterationsArg("i", "iterations", "Number of updates each sample performs", false, 100, "int");
		TCLAP::ValueArg<int> seedArg("S", "seed", "Positive integer to be used as seed for random number generation", false, -1, "int");
		TCLAP::SwitchArg useedArg("q", "unique-seed", "unique seed per rank", true);
		TCLAP::SwitchArg reportParamsArg("", "report-params", "Enables parameter reporting for use with analysis scripts", false);
		TCLAP::ValueArg<std::string> distributionArg("d", "distribution", "Choose from: gaussian (default), empirical", false, "gaussian", "string");

		cmd.add(filepathArg);
		cmd.add(samplesArg);
		cmd.add(iterationsArg);
		cmd.add(seedArg);
     	cmd.add(useedArg);
		cmd.add(distributionArg);
		cmd.add(reportParamsArg);


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

		std::string distribution = distributionArg.getValue();
		// For nsamples, no specific range, only non-negative check
		setAndCheckValue(nsamples, samplesArg, "ERROR: Invalid number of samples\n", 0);

		// For niterations, same non-negative check
		setAndCheckValue(niterations, iterationsArg, "ERROR: Invalid number of iterations\n", 0);

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