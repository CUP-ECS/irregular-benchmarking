#!/bin/bash
set -e

export APP_NAME=CLAMR

export SOURCE_DIR=$(pwd)/..

# Setup data results store
export DIR=/p/lustre1/$(whoami)/BENCHMARK_RESULTS
# remove old application runs and benchmark binary (if applicable)
rm -rf ${DIR}/${APP_NAME} ${DIR}/benchmark
mkdir -p ${DIR}

# move benchmark binary into correct location
cp ../build/benchmark ${DIR}/

# move to working directory
cd ${DIR}

# Load proper gcc and openmpi
module load gcc/10.3.1 openmpi/4.1.2

# Iterate over job sizes (node count)
for i in {1,2,4,8,16}
do

    export NUM_NODES=$i
    export NUM_PROC_PER_NODE=32
    export NUM_PROCS=`expr $NUM_NODES \* $NUM_PROC_PER_NODE`
    export JOB_NAME=BENCHMARK

    export JOB_DIR=${DIR}/${APP_NAME}/${NUM_NODES}
    mkdir -p ${JOB_DIR}
    cp benchmark ${JOB_DIR}/
    cd ${JOB_DIR}

    export CONFIG_PATH=${SOURCE_DIR}/results/${APP_NAME}_QUARTZ_${NUM_NODES}_${NUM_PROC_PER_NODE}/BENCHMARK_CONFIG
    cp ${CONFIG_PATH} ${JOB_DIR}/
    
    echo "#!/bin/bash" > temp_sbatch
    echo "#SBATCH --job-name=${APP_NAME}" >> temp_sbatch
    echo "#SBATCH --nodes=${NUM_NODES}" >> temp_sbatch
    echo "#SBATCH --tasks-per-node=${NUM_PROC_PER_NODE}" >> temp_sbatch
    echo "#SBATCH --cpus-per-task=1" >> temp_sbatch
    echo "#SBATCH --time=1:00:00" >> temp_sbatch
    echo "#SBATCH --sockets-per-node=2" >> temp_sbatch
    echo "#SBATCH --cores-per-socket=18" >> temp_sbatch
    echo "#SBATCH --partition=pbatch" >> temp_sbatch
    echo "module load gcc/10.3.1 openmpi/4.1.2" >> temp_sbatch
    echo "mpirun -np ${NUM_PROCS} -npernode ${NUM_PROC_PER_NODE} ./benchmark -I 1000 -i 1000 --report-params -f ./BENCHMARK_CONFIG -d empirical > ${JOB_DIR}/results.txt" >> temp_sbatch
    echo "cd ${JOB_DIR}" >> temp_sbatch
    echo "rm -rf out*" >> temp_sbatch
    
    # Launch generated script
    sbatch temp_sbatch

    # Remove generated script
    rm temp_sbatch

done
