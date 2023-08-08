# MUST BE RUN FROM ROOT DIRECTORY
# (AKA THE PARENT DIRECTORY OF THIS ONE)

#!/bin/bash

declare -a APP_STRING_ARRAY=("CLAMR" "CABANAMD" "XRAGE")
SYSTEM="QUARTZ"
PROCS_PER_NODE="32"

for NODES in {1,2,4,8,16}
do
    FILES_TO_LOOK_AT=
    for APP in ${APP_STRING_ARRAY[@]}
    do
        FILE_STRING="./data/${APP}/${NODES}/${APP}_${SYSTEM}_${NODES}_${PROCS_PER_NODE}.txt"
        if [[ -f "${FILE_STRING}" ]]; then
            FILES_TO_LOOK_AT+="${FILE_STRING} "
        else
            echo "@ No such file: ${FILE_STRING}"
        fi

    done
    echo "@ Trying: ${FILES_TO_LOOK_AT}"
    python3 analysis.py --bin-count=100 --separate-results $FILES_TO_LOOK_AT
done
