# MUST BE RUN FROM ROOT DIRECTORY
# (AKA THE PARENT DIRECTORY OF THIS ONE)

#!/bin/bash

declare -a APP_STRING_ARRAY=("CLAMR" "CABANAMD" "XRAGE")

for i in {1,2,4,8,16}
do
    FILES_TO_LOOK_AT=
    for APP in ${APP_STRING_ARRAY[@]}
    do
        FILE_STRING="./data/${APP}/${i}/${APP}_QUARTZ_${i}_32.txt"
        if [[ -f "${FILE_STRING}" ]]; then
            FILES_TO_LOOK_AT+="${FILE_STRING} "
        else
            echo "@ No such file: ${FILE_STRING}"
        fi

    done
    echo "@ Trying: ${FILES_TO_LOOK_AT}"
    python3 analysis.py --bin-count=100 $FILES_TO_LOOK_AT
done
