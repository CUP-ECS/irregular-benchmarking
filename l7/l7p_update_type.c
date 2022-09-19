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

#define L7_LOCATION "L7_UPDATE_TYPE"
//#define _L7_DEBUG

/* Forward declarations of internal subroutines. */
static int create_recv_type(int recv_count, int init_offset, 
		            MPI_Datatype base_type, MPI_Datatype *send_type);
static int create_send_type(l7_id_database *l7_id_db, int send_count, int init_offset, 
		            MPI_Datatype base_type, MPI_Datatype *send_type);

int L7P_Update_Type_Create(
      l7_id_database            *l7_id_db, 
      const enum L7_Datatype    l7_datatype,
      struct l7_update_datatype *l7_update_datatype
      )
{
   /*
    * Purpose
    * =======
    * L7_Update_Type_Create created the type offset information needed for 
    * the in-place data scatter/gather (using neighbor collectives) in L7_Update.
    * It uses the topology information computed in L7_Setup to create MPI_Indexed_types 
    * that indicate the locations in the send and receive buffers to use for the 
    * neighbor collectives
    * 
    * Arguments
    * =========
    * 
    * l7_datatype        (input) const it
    *                    The base type of data that will be scatter/gathered and out
    *                    of which we'll construct the needed MPI datatypes 
    * 
    * l7_id              (input) const int
    *                    Handle to database containing communication requirements.
    * 
    * l7_update_datatype (output) struct *
    *                    Arrays of MPI Indexed_type datatypes for handling incoming
    *                    and outgoing data in L7_Update
    *
    * Notes:
    * =====
    * 1) Serial compilation creates a no-op
    * 
    */
#if defined HAVE_MPI
   int ierr, i, offset;
   
   MPI_Datatype mpi_type;

   int num_sends, num_recvs;

   /* Check sanity of input arguments */

   L7_ASSERT(l7_id_db != NULL, "l7_id_database not found.", -1);
   L7_ASSERT(l7_update_datatype != NULL, "l7_update_datatype == NULL.", -1);

   mpi_type = l7p_mpi_type(l7_datatype);
   num_sends = l7_id_db->num_sends;
   num_recvs = l7_id_db->num_recvs;

   l7_update_datatype->in_types = calloc(num_recvs, sizeof(MPI_Datatype)); 
   L7_ASSERT(l7_update_datatype->in_types != NULL, 
	     "Could not allocate space for update datatype in_types.", -1);
   l7_update_datatype->out_types = calloc(num_sends, sizeof(MPI_Datatype)); 
   L7_ASSERT(l7_update_datatype->out_types != NULL, 
	     "Could not allocate space for update datatype out_types.", -1);

   /* Now that we've allocated the update datatype storage, create all of the 
    * types that go in it */

   /* Receives start right after the data this process owns */
   offset = l7_id_db->num_indices_owned;
   for (i = 0; i < num_recvs; i++) {
      int msg_count = l7_id_db->recv_counts[i];
#if defined _L7_DEBUG
      printf("[pe %d] Constructing recv type %d (%d elements at offset %d) from [pe %d].\n",
             l7.penum, i, msg_count, offset, l7_id_db->recv_from[i]);
#endif
      ierr = create_recv_type(msg_count, offset, 
			      mpi_type, &l7_update_datatype->in_types[i]);
      L7_ASSERT(ierr == 0, "Failed to create update recv datatype.", ierr);

      offset += msg_count;
   }

   offset = 0;
   for (i=0; i < num_sends; i++){
      int msg_count = l7_id_db->send_counts[i];
#if defined _L7_DEBUG
      printf("[pe %d] Constructing send type %d (%d elements) to [pe %d].\n",
             l7.penum, i, msg_count, l7_id_db->send_to[i]);
#endif
      ierr = create_send_type(l7_id_db, msg_count, offset,
                              mpi_type, &l7_update_datatype->out_types[i]);
      L7_ASSERT(ierr == 0, "Failed to create update send datatype.", ierr);

      offset += msg_count;
   }
#endif

   return(L7_OK);
}

int 
L7P_Update_Type_Free(l7_id_database *l7_id_db, 
		      struct l7_update_datatype *l7_update_datatype) 
{
#ifdef HAVE_MPI
   int num_sends, num_recvs;

   L7_ASSERT(l7_update_datatype != NULL, "l7_update_datatype == NULL", -1);
   L7_ASSERT(l7_id_db != NULL, "l7_id_database not found.", -1);

   num_sends = l7_id_db->num_sends;
   num_recvs = l7_id_db->num_recvs;

   for (int i = 0; i < num_sends; i++) 
   {
	MPI_Type_free(&l7_update_datatype->out_types[i]);
   }
   free(l7_update_datatype->out_types);
   l7_update_datatype->out_types = NULL;

   for (int i = 0; i < num_recvs; i++) 
   {
	MPI_Type_free(&l7_update_datatype->in_types[i]);
   }
   free(l7_update_datatype->in_types);
   l7_update_datatype->in_types = NULL;

#endif /* HAVE_MPI */

   return(L7_OK);
}

#ifdef HAVE_MPI

/* For the receive type, we just use the converted L7 base type and specify
 * the offset and length to MPI_Neighbor_alltoallw. Alternatively, we could
 * create a more complex type here if we wanted to scatter the data on 
 * receive */ 
static int
create_recv_type(int recv_count, int init_offset, 
		 MPI_Datatype base_type, MPI_Datatype *recv_type)
{
   int length[1], offset[1];

   length[0] = recv_count;
   offset[0] = init_offset;

#if defined _L7_DEBUG
      printf("[pe %d]     Recv type has %d elements starting at array offset %d.\n", 
             l7.penum, length[0], offset[0]);
#endif

   MPI_Type_indexed(1, length, offset, base_type, recv_type);
   MPI_Type_commit(recv_type);

#if defined _L7_DEBUG
   int lb, extent;
   MPI_Type_get_true_extent(*recv_type, (MPI_Aint *)&lb, (MPI_Aint *)&extent);
#endif /* _L7_DEBUG */
   return(L7_OK);
}

static int
create_send_type(l7_id_database *l7_id_db, int send_count, int init_offset, 
		 MPI_Datatype base_type, MPI_Datatype *send_type)
{
   int num_blocks = 0;
   int last_index = -2;
   int i, offset, blockidx;

   int *block_lens = NULL,
       *block_offsets = NULL;

   /* How many blocks will the indexed type need? */
   for (i = 0, offset = init_offset; i < send_count; i++, offset++)
   {
      int curr_index = l7_id_db->indices_local_to_send[offset];
      if (curr_index != last_index + 1) {
         num_blocks++;
      }
      last_index = curr_index;
   }

   /* Now that we know the number of blocks in the datatype, allocate
    * the lists of them and fill them out. */
   block_lens = calloc(num_blocks, sizeof(int));
   L7_ASSERT(block_lens != NULL, 
	     "Could not allocate space for type block lengths.", -1);
   block_offsets = calloc(num_blocks, sizeof(int));
   L7_ASSERT(block_offsets != NULL, 
	     "Could not allocate space for type block offsets.", -1);

   last_index = -2;
   blockidx = -1;
   for (i = 0, offset = init_offset; i < send_count; i++, offset++)
   {
      int curr_index = l7_id_db->indices_local_to_send[offset];
      if (curr_index != last_index + 1) {
         blockidx++;
         block_offsets[blockidx] = curr_index;
      }
      last_index = curr_index;
      block_lens[blockidx]++;
   }

#if defined _L7_DEBUG
   printf("[pe %d]     Send type has %d elements in %d blocks.\n", l7.penum,
          send_count, num_blocks);
   for (int i = 0; i < num_blocks; i++) {
      printf("[pe %d]         Block %d of length %d starts at offset %d.\n",
             l7.penum, i, block_lens[i], block_offsets[i]);
   }
#endif

   MPI_Type_indexed(num_blocks, block_lens, block_offsets, base_type, send_type);
   MPI_Type_commit(send_type);

   free(block_lens);
   free(block_offsets);

   return(L7_OK);
}

#endif /* HAVE_MPI */
