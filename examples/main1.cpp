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
#include "vernier.h"
#include <unistd.h>

void test_matrix(const char* filename,const char* name)
{
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Read suitesparse matrix
    ParMat<int> A;
    int idx;
    readParMatrix(filename, A);

    std::string formCommString = std::string(name) + "_form_comm";


    begin_pattern(formCommString.c_str());
    	form_comm(A);
    end_pattern();
    std::vector<int> pmpi_recv_vals, mpix_recv_vals;
    std::vector<int> send_vals, alltoallv_send_vals;
    std::vector<long> send_indices;

    if (A.on_proc.n_cols)
    {
        send_vals.resize(A.on_proc.n_cols);
        std::iota(send_vals.begin(), send_vals.end(), 0);
        for (int i = 0; i < A.on_proc.n_cols; i++)
        {
            send_vals[i] += (rank * 1000);
        }
    }

    if (A.recv_comm.size_msgs)
    {
        pmpi_recv_vals.resize(A.recv_comm.size_msgs);
        mpix_recv_vals.resize(A.recv_comm.size_msgs);
    }

    if (A.send_comm.size_msgs)
    {
        alltoallv_send_vals.resize(A.send_comm.size_msgs);
        send_indices.resize(A.send_comm.size_msgs);
        for (int i = 0; i < A.send_comm.size_msgs; i++)
        {
            idx                    = A.send_comm.idx[i];
            alltoallv_send_vals[i] = send_vals[idx];
            send_indices[i]        = A.send_comm.idx[i] + A.first_col;
        }
    }
  std::string communicateString = std::string(name) + "_communicate";
 	begin_pattern(communicateString.c_str());
    	communicate(A, send_vals, mpix_recv_vals, MPI_INT);
    end_pattern();


}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);






    test_matrix("/g/g20/bacon4/spackenvs/spackUNM26/localityaware/test_data/dwt_162.pm","dwt_162.pm");
    test_matrix("/g/g20/bacon4/sparse/pm/cage15.pm","cage15.pm");
    test_matrix("/g/g20/bacon4/sparse/pm/dielFilterV2real.pm","dielFilterV2real.pm");
    test_matrix("/g/g20/bacon4/sparse/pm/dielFilterV3real.pm","dielFilterV3real.pm");
    test_matrix("/g/g20/bacon4/sparse/pm/Flan_1565.pm","Flan_1565.pm");
    test_matrix("/g/g20/bacon4/sparse/pm/Geo_1438.pm","Geo_1438.pm");
    test_matrix("/g/g20/bacon4/sparse/pm/Hook_1498.pm","Hook_1498.pm");

    flush();
    MPI_Barrier(MPI_COMM_WORLD);
    std::this_thread::sleep_for(std::chrono::seconds(100));
    flush();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    MPI_Barrier(MPI_COMM_WORLD);
    sleep(400);





    MPI_Finalize();
    return 0;
}