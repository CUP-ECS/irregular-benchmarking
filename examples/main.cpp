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



enum distribution {
    GAUSSIAN,
    EMPIRICAL
 };

typedef enum distribution distribution_t;

enum prefix {
    A,B,K,M,G
};

typedef enum prefix prefix_t;

static int typesize = 8;
static int numpes = 0;
static int nsamples = 25;
static int niterations = 100;
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
static int unit_div = 1;
static prefix unit_symbol = A;
static std::string filepath = "";
static distribution_t distribution_type = GAUSSIAN;
static bool irregularity = 1;
static bool irregularity_owned = 1;
static bool irregularity_neighbors = 1;
static bool irregularity_stride = 1;
static bool irregularity_blocksz = 1;
static bool irregularity_remote = 1;
static bool report_params = 0;
static int seed = -1;


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


int empirical_dist(const char* param) {
//    // set the parameter string to look for.
    char param_key[25] = "PARAM: ";
    strcat(param_key, param);

    return -1;
}


void migrationExample()
{

  int nowned_orig = nowned;
  int nneighbors_orig = nneighbors;
  int nremote_orig = nremote;
  int blocksz_orig = blocksz;
  int stride_orig = stride;

  for(int sample_iter = 0; sample_iter < nsamples+1; sample_iter++) {

    nowned = gauss_dist(nowned_orig, nowned_stdv);
    nremote = gauss_dist(nremote_orig, nremote_orig);
    blocksz = gauss_dist(blocksz_orig, blocksz_stdv);
    nneighbors = gauss_dist(nneighbors_orig, nneighbors_stdv);
    stride = gauss_dist(stride_orig, stride_stdv);

    if (1) {
      printf("PARAM: nowned - %d\n", nowned);
      printf("PARAM: nremote - %d\n", nremote);
      printf("PARAM: blocksize - %d\n", blocksz);
      printf("PARAM: stride - %d\n", stride);
      printf("PARAM: nneighbors - %d\n", nneighbors);
    }





    int comm_rank = -1;
    MPI_Comm_rank( MPI_COMM_WORLD, &comm_rank );
    int comm_size = -1;
    MPI_Comm_size( MPI_COMM_WORLD, &comm_size );


    int remainder = nneighbors % 2;
    int num_partners_lo, num_partners_hi;
    int offset;
    std::vector<int> partner_pe(nneighbors);


    // Adjust the number of partners based on comm_rank
    if (comm_rank < (nneighbors / 2)) {
      num_partners_lo = nneighbors / 2;
      num_partners_hi = nneighbors / 2 + remainder;
    } else {
      num_partners_lo = nneighbors / 2 + remainder;
      num_partners_hi = nneighbors / 2;
    }

    // Partners below the current rank
    offset = 0;

    for (int i=1; i <= num_partners_lo; i++) {
      int partner = (i > comm_rank) ? (comm_size+ comm_rank - i) : comm_size - 1;
      //printf("[pe %d] offset %d penum %d i %d \n",penum, offset, penum, i);
      partner_pe[offset] = partner;
      offset++;
    }

    /* Indices above this PE */
    for (int i=1; i<=num_partners_hi; i++){
      int partner = (i + comm_rank >= comm_size) ? i + comm_rank - comm_size : i + comm_rank;
      //printf("[pe %d] offset %d penum %d i %d \n",penum, offset, penum, i);
      partner_pe[offset] = partner;
      offset++;
    }




    // Output the partners
    for (int i = 0; i < partner_pe.size(); i++) {
      std::cout << "Partner " << i << ": " << partner_pe[i] << std::endl;
    }

    std::vector<int> needed_indices(nremote);
    int  num_indices_per_partner;
    if (nneighbors != 0) {
      num_indices_per_partner = nremote / nneighbors;
    }
    else {
      nremote = 0;
      num_indices_per_partner = 0;
    }
    printf("nowned: %d\n", nowned);




    std::cout << "Contents of needed_indices: ";
    for (int i = 0; i < nremote; ++i) {
      std::cout << needed_indices[i] << " ";
    }

    std::cout << std::endl;



    /*
      Declare the AoSoA parameters.
    */
    using DataTypes = Cabana::MemberTypes<int, int>;
    const int VectorLength = 8;
    using MemorySpace = Kokkos::HostSpace;


    int num_tuple = nowned;
    Cabana::AoSoA<DataTypes, MemorySpace, VectorLength> aosoa( "A", num_tuple );

    auto slice_ranks = Cabana::slice<0>( aosoa );
    auto slice_ids = Cabana::slice<1>( aosoa );
    for ( int i = 0; i < num_tuple; ++i )
    {
        slice_ranks( i ) = comm_rank;
        slice_ids( i ) = i+(num_tuple*comm_rank);
    }




////    if ( comm_rank == 0 )
//    {
//        std::cout << "BEFORE migration" << std::endl
//                  << "(Rank " << comm_rank << ") ";
//        for ( std::size_t i = 0; i < slice_ranks.size(); ++i )
//            std::cout << slice_ranks( i ) << " ";
//        std::cout << std::endl
//                  << "(" << slice_ranks.size() << " ranks before migrate)"
//                  << std::endl
//                  << "(Rank " << comm_rank << ") ";
//        for ( std::size_t i = 0; i < slice_ids.size(); ++i )
//            std::cout << slice_ids( i ) << " ";
//        std::cout << std::endl
//                  << "(" << slice_ids.size() << " IDs before migrate)"
//                  << std::endl
//                  << std::endl;
//    }
//

    Kokkos::View<int*, MemorySpace> export_ranks( "export_ranks", num_tuple );



    for ( int i = 0; i < num_tuple; ++i )
        export_ranks( i ) = comm_rank;


    int num_indices_offpe = 0;


    for (int i=0; i<nneighbors; i++) {
      int  inum = 0;

      for (int j=0, k = 0; j<num_indices_per_partner; j++, k++) {
        /* Detect end of block */
        if (k >= blocksz) {
          inum += (1 + stride);
          k = 0;
        } else {
          inum++;
        }

        /* Detect if we would walk off the end of the remote */
        if (inum >= nowned)
          break;

        export_ranks(inum ) = partner_pe[i];
      }
    }






    std::vector<int> neighbors = { previous_rank, comm_rank, next_rank };
    std::sort( neighbors.begin(), neighbors.end() );
    auto unique_end = std::unique( neighbors.begin(), neighbors.end() );
    neighbors.resize( std::distance( neighbors.begin(), unique_end ) );
    Cabana::Distributor<MemorySpace> distributor( MPI_COMM_WORLD, export_ranks,
                                                  neighbors );



    Cabana::AoSoA<DataTypes, MemorySpace, VectorLength> destination(
        "destination", distributor.totalNumImport() );

    Cabana::migrate( distributor, aosoa, destination );

    auto slice_ranks_dst = Cabana::slice<0>( destination );
    auto slice_ids_dst = Cabana::slice<1>( destination );
    Cabana::migrate( distributor, slice_ranks, slice_ranks_dst );
    Cabana::migrate( distributor, slice_ids, slice_ids_dst );

    Cabana::migrate( distributor, aosoa );


    slice_ranks = Cabana::slice<0>( aosoa );
    slice_ids = Cabana::slice<1>( aosoa );

////    if ( comm_rank == 0 )
//    {
//        std::cout << "AFTER migration" << std::endl
//                  << "(Rank " << comm_rank << ") ";
//        for ( std::size_t i = 0; i < slice_ranks.size(); ++i )
//            std::cout << slice_ranks( i ) << " ";
//        std::cout << std::endl
//                  << "(" << slice_ranks.size() << " ranks after migrate)"
//                  << std::endl
//                  << "(Rank " << comm_rank << ") ";
//        for ( std::size_t i = 0; i < slice_ids.size(); ++i )
//            std::cout << slice_ids( i ) << " ";
//        std::cout << std::endl
//                  << "(" << slice_ids.size() << " IDs after migrate)"
//                  << std::endl;
//    }
    }
}






void parse_config_file(std::string config_file) {
  std::ifstream file(config_file);

  if (!file.is_open()) {
    std::cerr << "Error: Could not open file!" << std::endl;
  } else {
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();
    nlohmann::json j = nlohmann::json::parse(input);
//     Accessing the data
    for (const auto& param : j["parameters"]) {



      std::string name = param["name"].get<std::string>();



      int mean =  param["mean"].get<int>();



      int stddev = param["stdev"].get<int>();


      if (name == "nowned") {
          nowned=mean;
          nowned_stdv = stddev;
      } else if (name == "nremote") {
          nremote=mean;
          nremote_stdv = stddev;
      } else if (name == "blocksize") {
          blocksz=mean;
          blocksz_stdv = stddev;
      } else if (name == "comm_partners") {
          nneighbors=mean;
          nneighbors_stdv = stddev;
      }else if (name == "stride") {
          stride=mean;
          stride_stdv = stddev;
      }else {
      //todo
      }


    }
  }



}

bool setValue(){
  return true;
}

void exitError(const std::string& error_message) {
    std::cerr << error_message << std::flush; // Use std::cerr for error messages

    std::exit(-1); // Exit the program with error code
}


void setAndCheckValue(int& value, TCLAP::ValueArg<int>& arg, const char* errorMessage, int minValue = 0, int maxValue = INT_MAX) {
  int tempValue = arg.getValue();

  if (tempValue != -1) {
    value = tempValue;
  }

  // General value check
  if (value < minValue || value > maxValue) {
    exitError(errorMessage);
  }
}


void parseArgs(int argc, char **argv){

  try {
        TCLAP::CmdLine cmd("\nNOTE: Setting parameters for the benchmark such as (neighbors, owned, remote, blocksize, and stride)"
                           "sets parameters to those values for the reference benchmark."
                           "Those parameters are then randomized for the irregular samples"
                           "where the user-set parameters become averages for the random generation."
                           "Use the `--disable-irregularity` flag to only run the reference benchmark.", ' ', "1.0");

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
        TCLAP::ValueArg<std::string> distributionArg("d", "distribution", "Choose from: gaussian (default), empirical", false, "gaussian", "string");
        TCLAP::ValueArg<std::string> unitsArg("u", "units", "Choose from: a,b,k,m,g (auto, bytes, kilobytes, etc.)", false, "auto", "string");


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
        cmd.add(disableirregularityArg);
        cmd.parse(argc, argv);


        filepath = filepathArg.getValue();
        bool config_file_used = false;//todo




    if (filepath != "NOFILE") {

      try {
        if (filepath.empty()) {
          std::cerr << "Filepath is empty!" << std::endl;
          return;
        }

        std::filesystem::path p(filepath);

        // Check if path exists and is a file
        if (std::filesystem::exists(p)) {
          parse_config_file(filepath);
        } else {
          exitError(  "The file does not exist.");
        }
      } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
      }

    }

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
            {"auto", {A, 1}},
            {"a", {A, 1}},
            {"bytes", {A, 1}},
            {"b", {A, 1}},
            {"kilobytes", {K, 1024}},
            {"k", {K, 1024}},
            {"megabytes", {M, 1024 * 1024}},
            {"m", {M, 1024 * 1024}},
            {"gigabytes", {G, 1024 * 1024 * 1024}},
            {"g", {G, 1024 * 1024 * 1024}}
        };




    auto it = unit_map.find(unit);
    if (it != unit_map.end()) {
      char unit_symbol = it->second.first;
      int unit_div = it->second.second;

      std::cout << "Unit Symbol: " << unit_symbol << std::endl;
      std::cout << "Unit Division: " << unit_div << std::endl;
    } else {
      exitError("ERROR: Invalid formatting choice [b, k, m, g]");
    }




        std::string distribution=distributionArg.getValue();

        if (distribution == "gaussian"  ||
            distribution == "g") {
          distribution_type = GAUSSIAN;

        }else if (distribution == "empirical"
                  || distribution == "e") {
          distribution_type=EMPIRICAL;
        }else {
          exitError("ERROR: Invalid distribution choice [empirical,gaussian]\n");
        }

       int seedholder=seedArg.getValue();
        if(seed!=-1&&seedholder==-1) {
          seed=time(NULL);
        }



        srand(seed);

        irregularity=disableirregularityArg.getValue();


        //        irregularity_owned
        //        irregularity_neighbors
        //        irregularity_stride
        //        irregularity_blocksz
        //        irregularity_remote
        //        report_params
















    }
    catch (TCLAP::ArgException &e) {
        std::cerr << "Error: " << e.error() << " for argument " << e.argId() << std::endl;
        exit(-1);
    }
}

int main(int argc, char** argv)
{


  MPI_Init( &argc, &argv );
  {



    parseArgs(argc, argv);
    Kokkos::ScopeGuard scope_guard( argc, argv );

    migrationExample();
  }
  MPI_Finalize();





	printf("Hi\n");
	return 0;
}
