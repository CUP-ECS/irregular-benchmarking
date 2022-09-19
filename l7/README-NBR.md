# Neighbor Collectives in L7 and CLAMR

## Overview 
This branch of CLAMR/L7 uses neighbor collectives to implement the L7 Update and Push_Update
operations instead of the direct isend/irecv calls used by prior versions. It also makes
very aggressive use of MPI derived datatypes, as neighbor collectives generally have to
have the send and receive patterns described that way.

The overall goals of this implementation are to:
  1. Provide an open application and abstraction layer that can be used to study the
     interface between higher-level application-scientist focused absrtractions and the
     low-level communication abstractions provided by MPI. In particular, the L7 update 
     and push_update primitives, which CLAMR uses extensively, are very similar in concept
     to the communication classes in the Trilinos/Tpetra distributor class and the Cabana 
     Halo and Distributor classes. In addition, other lab applications also apparently use 
     similar abstractions (e.g. the token library in the LANL xRage prodution code). 
  1. Provide a meaningful applcation in which neighbor collective and derived datatypes
     implemnetations and optimizations can be studied, particularly for handling
     irregular and dynamic communication requirements.  
  1. Provide a platform for studying new MPI abstractions that address the performance
     problems faced by modern HPC applications, including managing communication couplings
     between multiple GPUs and CPUs, optimizing data movement in hybrid systems, 
     GPU triggering of communication operations.

## Current Status
The current implmentation includes a basic implementation of update and push_update using
neighbor_alltoallw and MPI_Type_indexed.The implementation has not been optimized or 
even carefully cleaned up. The degree to which the current update and push_update support
GPUs on CUDA-aware MPI implmenetations is also not clear.

### Short Term TODO items
  1. Remove Setup and Push_setup state no longer needed after communicator and type 
     creation.
  1. Clean up push interface, including to specify update type and callability from FORTRAN
  1. Test usage of Update and Push_Update with data stored on GPUs

### Medium Term Goal/Opportunities
  1. Create better L7 update and push_update micro-benchmarks for initial testing.
  1. Examine the impact of the 2019 IPDPS topology-aware neighbor collective optimizations 
     on L7 and CLAMR performance ("An Efficient Collaborative Communication
     Mechanism for MPI Neighborhood Collectives", by Ghazimirsaeed et al.) Prof. Bridges
     has a copy of the source code implemneted in MVAPICH to test.
  1. No weights are currently passed to the graph communiator creation algorithm even 
     though communication intensity data *is* available (the number of items that will be 
     communicated on each edge) at creation time. Examine adding this information to inform
     neighbor optimizations.
  1. Examine the use of persistent collectives for handling L7 Update and Push_Update.

### Long Term Research Questions
  1. How would an interface like L7 integrate with something like finepoints to reduce 
     coupling and increase application parallelism during communication?
