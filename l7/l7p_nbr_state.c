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

#define L7_LOCATION "L7_NBR_STATE"

int l7p_nbr_state_free(
		struct nbr_state *nbr_state
		)
{
	/* Free neighbor collective state set up for previous version. */
	if (l7.numpes > 1){
            MPI_Comm_free(&nbr_state->comm);
        }
        if (nbr_state->mpi_recv_counts) {
            free(nbr_state->mpi_recv_counts); 
            nbr_state->mpi_recv_counts = NULL;
        } 
        if (nbr_state->mpi_send_counts) {
            free(nbr_state->mpi_send_counts); 
            nbr_state->mpi_send_counts = NULL;
        }

        if (nbr_state->mpi_recv_offsets) {
            free(nbr_state->mpi_recv_offsets); 
            nbr_state->mpi_recv_offsets = NULL;
        }
        if (nbr_state->mpi_send_offsets) {
           free(nbr_state->mpi_send_offsets); 
           nbr_state->mpi_send_offsets = NULL;
	}
	
	return(L7_OK);
}

int l7p_nbr_state_create(
		struct nbr_state *nbr_state,
		int num_recvs,
		int num_sends
		)
{
	int i;

        nbr_state->mpi_recv_counts = calloc(num_recvs, sizeof(int)); 
        L7_ASSERT(nbr_state->mpi_recv_counts != NULL, 
     	     "Could not allocate space for mpi_recv_counts.", -1);
        nbr_state->mpi_send_counts = calloc(num_sends, sizeof(int)); 
        L7_ASSERT(nbr_state->mpi_send_counts != NULL, 
     	     "Could not allocate space for mpi_send_counts.", -1);

        nbr_state->mpi_recv_offsets = calloc(num_recvs, sizeof(long)); 
        L7_ASSERT(nbr_state->mpi_recv_offsets != NULL, 
     	     "Could not allocate space for mpi_recv_offsets.", -1);
        nbr_state->mpi_send_offsets = calloc(num_sends, sizeof(long)); 
        L7_ASSERT(nbr_state->mpi_send_offsets != NULL, 
     	     "Could not allocate space for mpi_send_offsets.", -1);

        for (i=0; i<num_recvs; i++){
           nbr_state->mpi_recv_counts[i] = 1;
           nbr_state->mpi_recv_offsets[i] = 0;
        }

        for (i=0; i<num_sends; i++){
           nbr_state->mpi_send_counts[i] = 1;
           nbr_state->mpi_send_offsets[i] = 0;
        }

	return L7_OK;
}
