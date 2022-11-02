#!/bin/bash
#SBATCH --job-name=Param_Analysis
#SBATCH --ntasks=40
#SBATCH --nodes=1
#SBATCH --cpus-per-task=4
#SBATCH --time=1-00:00:00
#SBATCH --output=%x.%j.log # Standard output and error log
#SBATCH --partition general

INPUT_FILE=./examples/reproducible_clamr.txt

echo "Loading Dependencies"
spack install python py-fitter
spack load python py-fitter

echo "Clearing Results"
cd ..
RESULTS_DIR=./results/
rm -rf ${RESULTS_DIR}
mkdir -p ${RESULTS_DIR}

echo "Starting Analysis"
python3 analysis.py ${INPUT_FILE}
echo "Analysis Done"

