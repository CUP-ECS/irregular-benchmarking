#include <assert.h>

#include <math.h>

#include <mpi.h>

#include <stdlib.h>

#include <thread>

#include <chrono>

#include <iostream>

#include <numeric>

#include <set>

#include <vector>

#include "locality_aware.h"

#include "/g/g20/bacon4/spackenvs/spackUNM26/localityaware/library/tests/tests/par_binary_IO.hpp"

#include "/g/g20/bacon4/spackenvs/spackUNM26/localityaware/library/tests/tests/sparse_mat.hpp"
 //#include "vernier.h"
#include <unistd.h>

#include <iostream>

#include <chrono>

#include <Cabana_Core.hpp>

#include <Kokkos_Core.hpp>


#define PRINT_LINE()                         \
do {                                    \
} while (0)
//printf("Line: start %d\n", __LINE__);     \
//fflush(stdout);                     \
//MPI_Barrier(MPI_COMM_WORLD);        \
//printf("Line: passed %d\n", __LINE__);     \
//fflush(stdout);                     \
} while (0)
//void test_matrix(const char* filename,const char* name)
//{
//    int rank, num_procs;
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
//
//    // Read suitesparse matrix
//    ParMat<int> A;
//    int idx;
//    readParMatrix(filename, A);
//
//    std::string formCommString = std::string(name) + "_form_comm";
//
//
//    begin_pattern(formCommString.c_str());
//    	form_comm(A);
//    end_pattern();
//    std::vector<int> pmpi_recv_vals, mpix_recv_vals;
//    std::vector<int> send_vals, alltoallv_send_vals;
//    std::vector<long> send_indices;
//
//    if (A.on_proc.n_cols)
//    {
//        send_vals.resize(A.on_proc.n_cols);
//        std::iota(send_vals.begin(), send_vals.end(), 0);
//        for (int i = 0; i < A.on_proc.n_cols; i++)
//        {
//            send_vals[i] += (rank * 1000);
//        }
//    }
//
//    if (A.recv_comm.size_msgs)
//    {
//        pmpi_recv_vals.resize(A.recv_comm.size_msgs);
//        mpix_recv_vals.resize(A.recv_comm.size_msgs);
//    }
//
//    if (A.send_comm.size_msgs)
//    {
//        alltoallv_send_vals.resize(A.send_comm.size_msgs);
//        send_indices.resize(A.send_comm.size_msgs);
//        for (int i = 0; i < A.send_comm.size_msgs; i++)
//        {
//            idx                    = A.send_comm.idx[i];
//            alltoallv_send_vals[i] = send_vals[idx];
//            send_indices[i]        = A.send_comm.idx[i] + A.first_col;
//        }
//    }
//  std::string communicateString = std::string(name) + "_communicate";
// 	begin_pattern(communicateString.c_str());
//    	communicate(A, send_vals, mpix_recv_vals, MPI_INT);
//    end_pattern();
//
//
//}

void time_matrix(const char * filename, const char * name) {
  int rank, num_procs;
  MPI_Comm_rank(MPI_COMM_WORLD, & rank);
  MPI_Comm_size(MPI_COMM_WORLD, & num_procs);

  // Read suitesparse matrix
  ParMat < int > A;
  int idx;
  readParMatrix(filename, A);

  form_comm(A);

  std::vector < int > pmpi_recv_vals, mpix_recv_vals;
  std::vector < int > send_vals, alltoallv_send_vals;
  std::vector < long > send_indices;

  if (A.on_proc.n_cols) {
    send_vals.resize(A.on_proc.n_cols);
    std::iota(send_vals.begin(), send_vals.end(), 0);
    for (int i = 0; i < A.on_proc.n_cols; i++) {
      send_vals[i] += (rank * 1000);
    }
  }

  if (A.recv_comm.size_msgs) {
    pmpi_recv_vals.resize(A.recv_comm.size_msgs);
    mpix_recv_vals.resize(A.recv_comm.size_msgs);
  }

  if (A.send_comm.size_msgs) {
    alltoallv_send_vals.resize(A.send_comm.size_msgs);
    send_indices.resize(A.send_comm.size_msgs);
    for (int i = 0; i < A.send_comm.size_msgs; i++) {
      idx = A.send_comm.idx[i];
      alltoallv_send_vals[i] = send_vals[idx];
      send_indices[i] = A.send_comm.idx[i] + A.first_col;
    }
  }

  auto TIME_START = std::chrono::high_resolution_clock::now();
  communicate(A, send_vals, mpix_recv_vals, MPI_INT);
  auto TIME_END_1 = std::chrono::high_resolution_clock::now();
  for (int i = 1; i < 10; i++) {
    communicate(A, send_vals, mpix_recv_vals, MPI_INT);
  }
  auto TIME_END_10 = std::chrono::high_resolution_clock::now();
  for (int i = 10; i < 25; i++) {
    communicate(A, send_vals, mpix_recv_vals, MPI_INT);
  }
  auto TIME_END_25 = std::chrono::high_resolution_clock::now();
  for (int i = 25; i < 100; i++) {
    communicate(A, send_vals, mpix_recv_vals, MPI_INT);
  }
  auto TIME_END_100 = std::chrono::high_resolution_clock::now();
  for (int i = 100; i < 1000; i++) {
    communicate(A, send_vals, mpix_recv_vals, MPI_INT);
  }
  auto TIME_END_1000 = std::chrono::high_resolution_clock::now();

  // Convert to milliseconds
  double t1 = std::chrono::duration < double, std::milli > (TIME_END_1 - TIME_START).count();
  double t10 = std::chrono::duration < double, std::milli > (TIME_END_10 - TIME_START).count();
  double t25 = std::chrono::duration < double, std::milli > (TIME_END_25 - TIME_START).count();
  double t100 = std::chrono::duration < double, std::milli > (TIME_END_100 - TIME_START).count();
  double t1000 = std::chrono::duration < double, std::milli > (TIME_END_1000 - TIME_START).count();

  double local_times[5] = {
    t1,
    t10,
    t25,
    t100,
    t1000
  };
  double min_times[5];
  double max_times[5];
  double sum_times[5];

  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, & rank);
  MPI_Comm_size(MPI_COMM_WORLD, & size);

  // Compute MIN
  MPI_Reduce(local_times, min_times, 5, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

  // Compute MAX
  MPI_Reduce(local_times, max_times, 5, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // Compute SUM (for average)
  MPI_Reduce(local_times, sum_times, 5, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {

    const char * labels[5] = {
      "1",
      "10",
      "25",
      "100",
      "1000"
    };
    printf("MPI file %s -  ranks: %d  \n", name, size);
    for (int i = 0; i < 5; i++) {
      double avg = sum_times[i] / size;

      printf("Iterations %s: ", labels[i]);
      printf("   Min: %.6f ms", min_times[i]);
      printf("   Max: %.6f ms ", max_times[i]);
      printf("   Avg: %.6f ms \n", avg);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

void cabana_matrix(const char * filename, const char * name) {
        Kokkos::Profiling::ScopedRegion region( "cabana_matrix" );

  int rank, num_procs;
  MPI_Comm_rank(MPI_COMM_WORLD, & rank);
  MPI_Comm_size(MPI_COMM_WORLD, & num_procs);

  // Read suitesparse matrix
  ParMat < int > A;
  int idx;
  readParMatrix(filename, A);

  form_comm(A);

  std::vector < int > pmpi_recv_vals, mpix_recv_vals;
  std::vector < int > send_vals, alltoallv_send_vals;
  std::vector < long > send_indices;

  if (A.on_proc.n_cols) {
    send_vals.resize(A.on_proc.n_cols);
    std::iota(send_vals.begin(), send_vals.end(), 0);
    for (int i = 0; i < A.on_proc.n_cols; i++) {
      send_vals[i] += (rank * 1000);
    }
  }

  if (A.recv_comm.size_msgs) {
    pmpi_recv_vals.resize(A.recv_comm.size_msgs);
    mpix_recv_vals.resize(A.recv_comm.size_msgs);
  }
 using DataTypes = Cabana::MemberTypes <int> ;
 const int VectorLength = 2;
 using MemorySpace = Kokkos::HostSpace;
PRINT_LINE();

  int num_tuple = A.on_proc.n_cols*6;
  Cabana::AoSoA < DataTypes, MemorySpace, VectorLength > aosoa("my_aosoa", num_tuple);
  auto slice_ranks = Cabana::slice < 0 > (aosoa);
  //   auto slice_ids = Cabana::slice < 1 > (aosoa);
  for (int i = 0; i < num_tuple; ++i) {
    slice_ranks(i) = i;
  }


int sendamount =0;
  for (int i = 0; i < A.send_comm.n_msgs; i++) {
    int proc = A.send_comm.procs[i]; // The destination rank for this chunk
    int start = A.send_comm.ptr[i]; // Start index in the 'idx' array
    int end = A.send_comm.ptr[i + 1]; // End index (exclusive)

    // Iterate through the elements belonging to this specific processor
    for (int j = start; j < end; j++) {


      sendamount++;
    }
  }

PRINT_LINE();
  Kokkos::View < int * , MemorySpace > export_ranks("export_ranks", sendamount);
  Kokkos::View < int * , MemorySpace > export_ids("export_ids", sendamount);
PRINT_LINE();
  for (int i = 0; i < sendamount; ++i) {
    export_ids(i) = -1;
    export_ranks(i) = -1;
  }
PRINT_LINE();
  if (A.send_comm.size_msgs) {

    alltoallv_send_vals.resize(A.send_comm.size_msgs);
    send_indices.resize(A.send_comm.size_msgs);
    for (int i = 0; i < A.send_comm.size_msgs; i++) {
      idx = A.send_comm.idx[i];
      alltoallv_send_vals[i] = send_vals[idx];
      send_indices[i] = A.send_comm.idx[i] + A.first_col;
    }
  }
PRINT_LINE();
  int q = 0;
  for (int i = 0; i < A.send_comm.n_msgs; i++) {
    int proc = A.send_comm.procs[i]; // The destination rank for this chunk
    int start = A.send_comm.ptr[i]; // Start index in the 'idx' array
    int end = A.send_comm.ptr[i + 1]; // End index (exclusive)

    // Iterate through the elements belonging to this specific processor
    for (int j = start; j < end; j++) {

      export_ids(q) = j;
      export_ranks(q) = proc;
      q++;
    }
  }
PRINT_LINE();
  Cabana::Halo < MemorySpace, Cabana::Export, Cabana::Mpi > halo(MPI_COMM_WORLD, num_tuple, export_ids, export_ranks);
PRINT_LINE();
  aosoa.resize(halo.numLocal() + halo.numGhost());
PRINT_LINE();
  slice_ranks = Cabana::slice < 0 > (aosoa);
PRINT_LINE();
  auto gather = Cabana::createGather(halo, aosoa, 1.0);
PRINT_LINE();
  auto TIME_START = std::chrono::high_resolution_clock::now();
  gather.apply();
  auto TIME_END_1 = std::chrono::high_resolution_clock::now();
  for (int i = 1; i < 10; i++) {
    gather.apply();
  }
  auto TIME_END_10 = std::chrono::high_resolution_clock::now();
  for (int i = 10; i < 25; i++) {
    gather.apply();
  }
  auto TIME_END_25 = std::chrono::high_resolution_clock::now();
  for (int i = 25; i < 100; i++) {
    gather.apply();
  }
  auto TIME_END_100 = std::chrono::high_resolution_clock::now();
  for (int i = 100; i < 1000; i++) {
    gather.apply();
  }
  auto TIME_END_1000 = std::chrono::high_resolution_clock::now();

  // Convert to milliseconds
  double t1 = std::chrono::duration < double, std::milli > (TIME_END_1 - TIME_START).count();
  double t10 = std::chrono::duration < double, std::milli > (TIME_END_10 - TIME_START).count();
  double t25 = std::chrono::duration < double, std::milli > (TIME_END_25 - TIME_START).count();
  double t100 = std::chrono::duration < double, std::milli > (TIME_END_100 - TIME_START).count();
  double t1000 = std::chrono::duration < double, std::milli > (TIME_END_1000 - TIME_START).count();

  double local_times[5] = {
    t1,
    t10,
    t25,
    t100,
    t1000
  };
  double min_times[5];
  double max_times[5];
  double sum_times[5];

  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, & rank);
  MPI_Comm_size(MPI_COMM_WORLD, & size);

  // Compute MIN
  MPI_Reduce(local_times, min_times, 5, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

  // Compute MAX
  MPI_Reduce(local_times, max_times, 5, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // Compute SUM (for average)
  MPI_Reduce(local_times, sum_times, 5, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {

    const char * labels[5] = {
      "1",
      "10",
      "25",
      "100",
      "1000"
    };
    printf("C file %s -  ranks: %d  \n", name, size);
    for (int i = 0; i < 5; i++) {
      double avg = sum_times[i] / size;

      printf("Iterations %s: ", labels[i]);
      printf("   Min: %.6f ms", min_times[i]);
      printf("   Max: %.6f ms ", max_times[i]);
      printf("   Avg: %.6f ms \n", avg);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
}



void runBoth(const char * filename, const char * name){
cabana_matrix( filename, name);
time_matrix( filename, name);
}


int main(int argc, char ** argv) {
  MPI_Init( & argc, & argv);
  Kokkos::ScopeGuard scope_guard(argc, argv);


  runBoth("/g/g20/bacon4/spackenvs/spackUNM26/localityaware/test_data/dwt_162.pm", "dwt_162.pm");
  runBoth("/g/g20/bacon4/sparse/pm/cage15.pm", "cage15.pm");
  runBoth("/g/g20/bacon4/sparse/pm/dielFilterV2real.pm", "dielFilterV2real.pm");
  runBoth("/g/g20/bacon4/sparse/pm/dielFilterV3real.pm", "dielFilterV3real.pm");
  runBoth("/g/g20/bacon4/sparse/pm/Flan_1565.pm", "Flan_1565.pm");
  runBoth("/g/g20/bacon4/sparse/pm/Geo_1438.pm", "Geo_1438.pm");
  runBoth("/g/g20/bacon4/sparse/pm/Hook_1498.pm", "Hook_1498.pm");

  //  flush();
  MPI_Barrier(MPI_COMM_WORLD);
  //std::this_thread::sleep_for(std::chrono::seconds(100));
  //flush();
  //std::this_thread::sleep_for(std::chrono::seconds(10));
  MPI_Barrier(MPI_COMM_WORLD);
  //sleep(400);

  MPI_Finalize();
  return 0;
}