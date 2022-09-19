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
 */  
#include "l7.h"
#include "l7p.h"

#include <stdlib.h>

#define L7_LOCATION "L7_UPDATE"
// #define _L7_DEBUG

int L7_Update(
      void                   *data_buffer,
      const enum L7_Datatype l7_datatype, 
      const int              l7_id
      )
{
   /*
    * Purpose
    * =======
    * L7_Update collects into array data_buffer data located off-process,
    * appending it to owned (on-process) data data_buffer. This version uses
    * MPI neighbor collectives and datatypes so the transfer is completely 
    * in-place, allowing accelerator-aware MPI versions to work properly and
    * in theory optimize data transfer and messaging orders to improve performance. 
    * Its performance, however, depends heavily on the performance of the underlying 
    * MPI doesn't carefully optimize MPI datatype handling and neighbor collectives
    * which some MPIs don't optimize well.
    * 
    * Arguments
    * =========
    * data_buffer        (input/output) void*
    *                    On input,
    *                    data_buffer[0:num_indices_owned-1] contains
    *                    data owned by process.
    *                    On output,
    *                    data_buffer[num_indices_owned, num_indices_needed-1]
    *                    contains the data collected from off-process.
    * 
    * l7_gather_datatype (input) L7_Gather_Datatype (a struct *)
    *                    The type of data contained in array data_buffer and the
    *                    the layout of the data to be scatter/gathered to other
    *                    processes.
    * 
    * l7_id              (input) const int
    *                    Handle to database containing communication requirements. Note
    *                    that this reuses the setup/update database as most of the data
    *                    is redundant instead of creating a full new type.
    * 
    * Notes:
    * =====
    * 1) Serial compilation creates a no-op
    * 
    */
#if defined HAVE_MPI
   
   /*
    * Local variables
    */
   int
     ierr;                 /* Error code for return              */
   l7_id_database
     *l7_id_db;            /* database associated with l7_id.    */
   int 
     sizeof_type;	   /* sizeof the L7 datatype */
   struct l7_update_datatype
     *update_datatype;     /* Info on the datatypes to scatter/gather */
     
   /*
    * Executable Statements
    */
   
   if (! l7.mpi_initialized){
      return(0);
   }
    
   if (l7.initialized !=1){
      ierr = 1;
      L7_ASSERT(l7.initialized == 1, "L7 not initialized", ierr);
   }
   
   /*
    * Check input.
    */
   
   if (data_buffer == NULL){
      ierr = -1;
      L7_ASSERT( data_buffer != NULL, "data_buffer != NULL", ierr);
   }
   
   sizeof_type = l7p_sizeof(l7_datatype);
   L7_ASSERT((sizeof_type > 0) && (sizeof_type <= 8), "Invalid L7 type in Update.", -1);
  
   if (l7_id <= 0){
      ierr = -1;
      L7_ASSERT( l7_id > 0, "l7_id <= 0", ierr);
   }
   
   if (l7.numpes == 1){
      ierr = L7_OK;
      return(ierr);
   }
   
   /*
    * Alias database associated with input l7_id
    */
   
   l7_id_db = l7p_set_database(l7_id);
   if (l7_id_db == NULL){
      ierr = -1;
      L7_ASSERT(l7_id_db != NULL, "Failed to find database.", ierr);
   }
   
   l7.penum = l7_id_db->penum;
   
   if (l7_id_db->numpes == 1){ /* No-op */
      ierr = L7_OK;
      return(ierr);
   }
   
   update_datatype = &l7_id_db->nbr_state.update_datatypes[sizeof_type];
   L7_ASSERT(update_datatype->in_types != NULL, "Invalid gather datatype.", -1);
   L7_ASSERT(update_datatype->out_types != NULL, "Invalid scatter datatype.", -1);

#if defined _L7_DEBUG
   printf("[pe %d] Update AllToAllW \n", l7.penum);
   for (int i = 0; i < l7_id_db->num_sends; i++) {
       printf("[pe %d]    send %d count %d offset %ld ",
	     l7.penum, i, l7_id_db->nbr_state.mpi_send_counts[i], l7_id_db->nbr_state.mpi_send_offsets[i]);
       printf("\n");
   }

   for (int i = 0; i < l7_id_db->num_recvs; i++) {
       printf("[pe %d]    recv %d count %d offset %ld ",
	     l7.penum, i, l7_id_db->nbr_state.mpi_recv_counts[i], l7_id_db->nbr_state.mpi_recv_offsets[i]);
       printf("\n");
   }

#endif /* _L7_DEBUG */
   
   /* Now that everything is all set up, neighbor_alltoallw does all of
    * the work (and data movement optimization) */
   ierr = MPI_Neighbor_alltoallw((void *)data_buffer, 
			  l7_id_db->nbr_state.mpi_send_counts,
			  (MPI_Aint *)l7_id_db->nbr_state.mpi_send_offsets, 
			  update_datatype->out_types, 
			  (void *)data_buffer, 
			  l7_id_db->nbr_state.mpi_recv_counts, 
			  (MPI_Aint *)l7_id_db->nbr_state.mpi_recv_offsets, 
			  update_datatype->in_types, 
			  l7_id_db->nbr_state.comm);
   
#endif /* HAVE_MPI */
   
   return(L7_OK);
    
} /* End L7_Update */

void L7_UPDATE(
      void                    *data_buffer,
      const enum L7_Datatype  *l7_datatype,
      const int               *l7_id
      )
{

    L7_Update(data_buffer, *l7_datatype, *l7_id);
}

int L7_Get_Num_Indices(const int l7_id)
{
   int ierr;

   l7_id_database
     *l7_id_db;            /* database associated with l7_id.    */

   if (l7_id <= 0){
      ierr = -1;
      L7_ASSERT( l7_id > 0, "l7_id <= 0", ierr);
   }

   l7_id_db = l7p_set_database(l7_id);
   if (l7_id_db == NULL){
      ierr = -1;
      L7_ASSERT(l7_id_db != NULL, "Failed to find database.", ierr);
   }

   int num_indices = 0;

   int num_sends = l7_id_db->num_sends;
   
   for (int i=0; i<num_sends; i++){
      /* count data to be sent. */
      
      num_indices += l7_id_db->send_counts[i];
   }

   return(num_indices);
}

int L7_Get_Local_Indices(const int l7_id, int *local_indices)
{
   int ierr;

   l7_id_database
     *l7_id_db;            /* database associated with l7_id.    */

   if (l7_id <= 0){
      ierr = -1;
      L7_ASSERT( l7_id > 0, "l7_id <= 0", ierr);
   }

   l7_id_db = l7p_set_database(l7_id);
   if (l7_id_db == NULL){
      ierr = -1;
      L7_ASSERT(l7_id_db != NULL, "Failed to find database.", ierr);
   }

   //int num_indices = 0;

   int num_sends = l7_id_db->num_sends;
   
   int offset = 0;

   for (int i=0; i<num_sends; i++){
      /* Copy data to be sent. */
      
      int send_count = l7_id_db->send_counts[i];
#ifdef _OPENMP_SIMD
#pragma omp simd
#endif
      for (int j=0; j<send_count; j++){
          local_indices[offset] = l7_id_db->indices_local_to_send[offset];
          offset++;
      }
   }

   return(0);
}
