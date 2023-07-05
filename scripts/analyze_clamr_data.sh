# MUST BE RUN FROM ROOT DIRECTORY
# (AKA THE PARENT DIRECTORY OF THIS ONE)

#!/bin/bash

for i in {1,2,4,8,16}
do
    python3 analysis.py --bin-count=100 ./data/CLAMR/${i}/CLAMR_QUARTZ_${i}_32.txt
done
