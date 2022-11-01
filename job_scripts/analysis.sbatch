#!/bin/bash
#SBATCH --job-name=Param_Analysis
#SBATCH --nodes=1
#SBATCH --time=4:00:00     # Time limit hrs:min:sec
#SBATCH --output=%x.%j.log # Standard output and error log
#SBATCH --partition general

INPUT_FILE=./examples/reproducible_clamr.txt

echo "Loading Dependencies"
spack load python py-matplotlib py-scipy py-numpy py-fitter

echo "Clearing Results"
cd ..
RESULTS_DIR=./results/
rm -rf ${RESULTS_DIR}
mkdir -p ${RESULTS_DIR}

echo "Starting Analysis"
python3 analysis.py ${INPUT_FILE}
echo "Analysis Done"

