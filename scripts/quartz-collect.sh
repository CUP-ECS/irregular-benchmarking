#!/bin/bash
set -e

# Run CLAMR
echo "Setting up CLAMR Runtime"


# Setup data results store
export DIR=/p/lustre1/$(whoami)/CLAMR
rm -rf ${DIR}
mkdir -p ${DIR}
cd ${DIR}

# Install instrumented CLAMR
spack install clamr graphics=none

# Iterate over job sizes (node count)
for i in {1,2,4,8,16}
do

    export NUM_NODES=$i
    export NUM_PROC_PER_NODE=32
    export JOB_NAME=CLAMR_${NUM_NODES}
    export JOBSIZE=$(echo "scale=1;2048*sqrt(${NUM_NODES})" | bc | cut -f 1 -d.)

    export JOB_DIR=${DIR}/${NUM_NODES}
    mkdir -p ${JOB_DIR}
    cd ${JOB_DIR}
    
    echo "Running ${JOB_NAME} with N=${JOBSIZE} on ${NUM_NODES} nodes."

    echo "#!/bin/bash" >> temp_sbatch
    echo "#SBATCH --job-name=${JOB_NAME}" >> temp_sbatch
    echo "#SBATCH --nodes=${NUM_NODES}" >> temp_sbatch
    echo "#SBATCH --tasks-per-node=${NUM_PROC_PER_NODE}" >> temp_sbatch
    echo "#SBATCH --cpus-per-task=1" >> temp_sbatch
    echo "#SBATCH --time=1:00:00" >> temp_sbatch
    echo "#SBATCH --sockets-per-node=2" >> temp_sbatch
    echo "#SBATCH --cores-per-socket=18" >> temp_sbatch
    echo "#SBATCH --partition=pbatch" >> temp_sbatch
    echo "spack load clamr" >> temp_sbatch
    echo "srun clamr_mpionly -n ${JOBSIZE} -l 2 -t 500 -i 100 > ${JOB_DIR}/CLAMR_QUARTZ_${NUM_NODES}_${NUM_PROC_PER_NODE}.txt" >> temp_sbatch
    echo "cd ${JOB_DIR}" >> temp_sbatch
    echo "rm out*" >> temp_sbatch
    
    # Launch generated script
    sbatch temp_sbatch

    # Remove generated script
    rm temp_sbatch

done


# Run CabanaMD
echo "Setting up CabanaMD Runtime"

# Install instrumeted CLAMR (assuming not already installed)
spack install cabanamd

# Setup data results store
export DIR=/p/lustre1/$(whoami)/CabanaMD
rm -rf ${DIR}
mkdir -p ${DIR}
cd ${DIR}

# Get fresh input file
git clone https://github.com/ECP-copa/CabanaMD.git
cp CabanaMD/input/in.lj in.lj
rm -rf CabanaMD

# Iterate over job sizes (node count)
for i in {1,2,4,8,16}
do
    export NUM_NODES=$i
    export NUM_PROC_PER_NODE=32
    export JOB_NAME=CBNMD_${NUM_NODES}
    
    echo "Running ${JOB_NAME} with N=${JOBSIZE} on ${NUM_NODES} nodes."

    echo "#!/bin/bash" >> temp_sbatch
    echo "#SBATCH --job-name=${JOB_NAME}" >> temp_sbatch
    echo "#SBATCH --nodes=${NUM_NODES}" >> temp_sbatch
    echo "#SBATCH --tasks-per-node=${NUM_PROC_PER_NODE}" >> temp_sbatch
    echo "#SBATCH --cpus-per-task=1" >> temp_sbatch
    echo "#SBATCH --time=1:00:00" >> temp_sbatch
    echo "#SBATCH --sockets-per-node=2" >> temp_sbatch
    echo "#SBATCH --cores-per-socket=18" >> temp_sbatch
    echo "#SBATCH --partition=pbatch" >> temp_sbatch
    echo "spack load cabanamd" >> temp_sbatch
    echo "srun cbnMD -il in.lj > ${DIR}/CABANAMD_QUARTZ_${NUM_NODES}_${NUM_PROC_PER_NODE}.txt" >> temp_sbatch

    # Launch generated script
    sbatch temp_sbatch
    
    # Remove generated script
    rm temp_sbatch

done

# TODO: Run ExaMPM


# TODO: Run MCMP


# TODO: Hyper


