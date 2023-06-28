# Scripts

Unless you have read the scripts and understand what is happening, I would recommend not using these scripts.
I have deliberately made them user-agnostic, but you will need to put data in the right places, and you might prefer a different workflow.

These scripts are data collection and benchmark running scripts to do tasks related to this project.
The `quartz-collect.sh` will iterate over various job sizes for CLAMR and CabanaMD and collect data into the `/p/lustre1/$(whoami)/` directory, with application data being put in a subdirectory of the same name.
As the name implies, this script will only run successfully on Quartz at LLNL.

You should then run the `analysis.py` script located in the root directory of this project on the data, and place the generated `BENCHMARK_CONFIG` file in the following location: `${SOURCE_DIR}/results/${APP_NAME}_QUARTZ_${NUM_NODES}_${NUM_PROC_PER_NODE}/`.

By adhering to this format, the `quartz-benchmark.sh` script will run the benchmark in the correct way to recreate a range of job sizes.
As the name implies, this script will also only run successfully on Quartz at LLNL.


