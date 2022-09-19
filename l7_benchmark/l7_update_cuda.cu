#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "l7/l7.h"

#include <cuda.h>
#include <cuda_runtime.h>

__global__ void
init_int_array(const int  num_indices_have, // 0
               const int  my_start_id,      // 1
               int *array)            // 2
{
   int i = blockIdx.x * blockDim.x + threadIdx.x;

   if (i >= num_indices_have) return;

   array[i] = i + my_start_id;
}

__global__ void
init_double_array(const int  num_indices_have, // 0
                  const int  my_start_id,      // 1
                  double *array)
{
   int i = blockIdx.x * blockDim.x + threadIdx.x;

   if (i >= num_indices_have) return;

   array[i] = i + my_start_id;
}

__global__ void
init_short_array(const int  num_indices_have, // 0
                 const int  my_start_id,      // 1
                 short *array)
{
   int i = blockIdx.x * blockDim.x + threadIdx.x;

   if (i >= num_indices_have) return;

   array[i] = i + my_start_id;
}


__global__ void
init_char_array(const int  num_indices_have, // 0
                const int  my_start_id,      // 1
                char *array)
{
   int i = blockIdx.x * blockDim.x + threadIdx.x;

   if (i >= num_indices_have) return;

   array[i] = i + my_start_id;
}

extern "C"
void initialize_data_cuda(void **odata, int nowned, int nremote, int typesize, int my_start_index)
{
   /*
    * Allocate data arrays on device and wait for initialization to complete
    */
   unsigned long data_size;
   data_size = nowned + nremote;
   switch (typesize) {
   case 1:
      cudaMalloc(odata, sizeof(char) * data_size);
      init_char_array<<<(nowned + 255)/256, 256>>>(nowned, my_start_index, *(char **)odata);
      break;
   case 2:
      cudaMalloc(odata, sizeof(short) * data_size);
      init_short_array<<<(nowned + 255)/256, 256>>>(nowned, my_start_index, *(short **)odata);
      break;
   case 4:
      cudaMalloc(odata, sizeof(int) * data_size);
      init_int_array<<<(nowned + 255)/256, 256>>>(nowned, my_start_index, *(int **)odata);
      break;
   case 8:
      cudaMalloc(odata, sizeof(double) * data_size);
      init_double_array<<<(nowned + 255)/256, 256>>>(nowned, my_start_index, *(double **)odata);
      break;
   }
   cudaDeviceSynchronize();
   return;
}
