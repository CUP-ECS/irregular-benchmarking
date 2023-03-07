#!/bin/bash

# Run CLAMR
echo "Setting up CLAMR Runtime"

# Install instrumeted CLAMR (assuming not already installed)
spack install clamr


# Setup data results store
export DIR=/p/lustre1/$(whoami)/CLAMR
rm -rf ${DIR}
mkdir -p ${DIR}
cd ${DIR}

# Iterate over CLAMR job sizes (node count)
for i in {1,2,4,8,16}
do
    
    echo "Setting up ${JOB_NAME} with ${NUM_NODES} nodes"
    
    export NUM_NODES=$i
    export JOB_NAME=CLAMR_${NUM_NODES}
    export JOBSIZE=$(echo "scale=1;2048*sqrt(${NUM_NODES})" | bc | cut -f 1 -d.)

    echo "Running ${JOB_NAME} with N=${JOBSIZE} on ${NUM_NODES} nodes."

    echo "#!/bin/bash" >> temp_sbatch
    echo "#SBATCH --job-name=${JOB_NAME}" >> temp_sbatch
    echo "#SBATCH --nodes=${NUM_NODES}" >> temp_sbatch
    echo "#SBATCH --tasks-per-node=32" >> temp_sbatch
    echo "#SBATCH --cpus-per-task=1" >> temp_sbatch
    echo "#SBATCH --time=1:00:00" >> temp_sbatch
    echo "#SBATCH --sockets-per-node=2" >> temp_sbatch
    echo "#SBATCH --cores-per-socket=18" >> temp_sbatch
    echo "#SBATCH --partition=pbatch" >> temp_sbatch
    echo "spack load clamr" >> temp_sbatch
    echo "srun clamr_mpionly -n ${JOBSIZE} -l 2 -t 500 -i 100 > ${DIR}/CLAMR_QUARTZ_${NUM_NODES}.txt" >> temp_sbatch

    # Launch generated script
    sbatch temp_sbatch

    # Remove generated script
    rm temp_sbatch

done


# TODO: Run CabanaMD


# TODO: Run ExaMPM


# TODO: Run MCMP


# TODO: Hyper


