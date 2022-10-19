/*
 *  Copyright (c) 2011-2019, Triad National Security, LLC.
 *  All rights Reserved.
 *
 *  CLAMR -- LA-CC-11-094
 *
 *  Copyright 2011-2019. Triad National Security, LLC. This software was produced
 *  under U.S. Government contract 89233218CNA000001 for Los Alamos National
 *  Laboratory (LANL), which is operated by Triad National Security, LLC
 *  for the U.S. Department of Energy. The U.S. Government has rights to use,
 *  reproduce, and distribute this software.  NEITHER THE GOVERNMENT NOR
 *  TRIAD NATIONAL SECURITY, LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR
 *  ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.  If software is modified
 *  to produce derivative works, such modified software should be clearly marked,
 *  so as not to confuse it with the version available from LANL.
 *
 *  Additionally, redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Triad National Security, LLC, Los Alamos
 *       National Laboratory, LANL, the U.S. Government, nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE TRIAD NATIONAL SECURITY, LLC AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 *  NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TRIAD NATIONAL
 *  SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  CLAMR -- LA-CC-11-094
 *  This research code is being developed as part of the
 *  2011 X Division Summer Workshop for the express purpose
 *  of a collaborative code for development of ideas in
 *  the implementation of AMR codes for Exascale platforms
 *
 *  AMR implementation of the Wave code previously developed
 *  as a demonstration code for regular grids on Exascale platforms
 *  as part of the Supercomputing Challenge and Los Alamos
 *  National Laboratory
 *
 *  Authors: Bob Robey       XCP-2   brobey@lanl.gov
 *           Neal Davis              davis68@lanl.gov, davis68@illinois.edu
 *           David Nicholaeff        dnic@lanl.gov, mtrxknight@aol.com
 *           Dennis Trujillo         dptrujillo@lanl.gov, dptru10@gmail.com
 *
 */

#ifdef HAVE_CL_DOUBLE
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
typedef double  real;
#else
typedef float   real;
#endif

__kernel void pack_short_have_data_cl(
                          const int   num_indices_have, // 0
                 __global       int   *indices_have,    // 1
                 __global       short *array,           // 2
                 __global       short *packed_data)     // 3
{
   const unsigned int giX  = get_global_id(0);

   if (giX >= num_indices_have) return;

   packed_data[giX] = array[indices_have[giX]];
}

__kernel void pack_int_have_data_cl(
                          const int  num_indices_have, // 0
                 __global       int  *indices_have,    // 1
                 __global       int  *array,           // 2
                 __global       int  *packed_data)     // 3
{
   const unsigned int giX  = get_global_id(0);

   if (giX >= num_indices_have) return;

   packed_data[giX] = array[indices_have[giX]];
}

__kernel void pack_float_have_data_cl(
                          const int   num_indices_have, // 0
                 __global       int   *indices_have,    // 1
                 __global       float *array,           // 2
                 __global       float *packed_data)     // 3
{
   const unsigned int giX  = get_global_id(0);

   if (giX >= num_indices_have) return;

   packed_data[giX] = array[indices_have[giX]];
}

__kernel void pack_double_have_data_cl(
                          const int    num_indices_have, // 0
                 __global       int    *indices_have,    // 1
                 __global       double *array,           // 2
                 __global       double *packed_data)     // 3
{
   const unsigned int giX  = get_global_id(0);

   if (giX >= num_indices_have) return;

   packed_data[giX] = array[indices_have[giX]];
}


__kernel void copy_ghost_short_data_cl(
                          const int   ncells,           // 0
                          const int   nghost,           // 1
                 __global       short *data_buffer,     // 2
                 __global       short *data_buffer_add) // 3
{
   const unsigned int giX  = get_global_id(0);

   if (giX >= nghost) return;

   data_buffer[ncells+giX] = data_buffer_add[giX];
}

__kernel void copy_ghost_int_data_cl(
                          const int  ncells,           // 0
                          const int  nghost,           // 1
                 __global       int  *data_buffer,     // 2
                 __global       int  *data_buffer_add) // 3
{
   const unsigned int giX  = get_global_id(0);

   if (giX >= nghost) return;

   data_buffer[ncells+giX] = data_buffer_add[giX];
}

__kernel void copy_ghost_float_data_cl(
                          const int   ncells,           // 0
                          const int   nghost,           // 1
                 __global       float *data_buffer,     // 2
                 __global       float *data_buffer_add) // 3
{
   const unsigned int giX  = get_global_id(0);

   if (giX >= nghost) return;

   data_buffer[ncells+giX] = data_buffer_add[giX];
}

__kernel void copy_ghost_double_data_cl(
                          const int    ncells,           // 0
                          const int    nghost,           // 1
                 __global       double *data_buffer,     // 2
                 __global       double *data_buffer_add) // 3
{
   const unsigned int giX  = get_global_id(0);

   if (giX >= nghost) return;

   data_buffer[ncells+giX] = data_buffer_add[giX];
}

