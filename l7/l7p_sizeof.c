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

int l7p_sizeof(
		const enum L7_Datatype  l7_datatype
		)
{
	/*
	 * Purpose
	 * =======
	 * l7p_sizeof returns the number of bytes needed to store
	 * one element of the input L7 datatype.
	 *
	 * Arguments
	 * =========
	 * l7_datatype     (input) const int*
	 *                 The type of data, as defined by L7 datatypes,
	 *                 in databuffer.
	 *
	 * Return value
	 * ============
	 * Values less than 0 indicates an error.
	 *
	 * Notes:
	 * ======
	 *
	 */

	/*
	 * Local variables.
	 */

	int
	  sizeof_type;  /* Number of bytes in one element of the input datatype. */

	/*
	 * Executable Statements
	 */

	switch (l7_datatype)
	{
	case L7_GENERIC8:
	case L7_BYTE:
	case L7_PACKED:
	case L7_LONG_LONG_INT:
	case L7_DOUBLE:
	case L7_INTEGER8:
	case L7_REAL8:
		sizeof_type = 8;
		break;
	case L7_INT:
	case L7_FLOAT:
	case L7_LOGICAL:
	case L7_INTEGER4:
	case L7_REAL4:
		sizeof_type = 4;
		break;
    case L7_SHORT:
        sizeof_type = 2;
        break;
    case L7_CHAR:
	case L7_CHARACTER:
        sizeof_type = 1;
        break;
	case L7_LONG:
		sizeof_type = sizeof(long);
		break;
	default:
	    sizeof_type = -1;
	    break;
	}

	return(sizeof_type);
}
