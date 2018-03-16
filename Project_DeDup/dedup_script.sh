#!/bin/bash
CSV=".csv"
LOG=".log"
SIZE="512"
KB="1024"
TOTALS="524288"
DISTANCE="0"
POINTERS="0"

for FILE in $( ls | grep ".csv" | cut -f 1 -d . ); do
    mkdir ${FILE}

    ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}KB_D${DISTANCE}_P${POINTERS}${LOG}"

    SIZE="8192"
    TOTALS=$((SIZE*KB))
    ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}KB_D${DISTANCE}_P${POINTERS}${LOG}"


    SIZE="524288"
    TOTALS=$((SIZE*KB))
    ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}KB_D${DISTANCE}_P${POINTERS}${LOG}"


    SIZE="8192"
    DISTANCE="2"
    POINTERS="0"
    TOTALS=$((SIZE*KB))
    ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}KB_D${DISTANCE}_P${POINTERS}${LOG}"


    SIZE="524288"
    DISTANCE="0"
    POINTERS="4"
    TOTALS=$((SIZE*KB))
    ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}KB_D${DISTANCE}_P${POINTERS}${LOG}"

    SIZE="8192"
    DISTANCE="3"
    POINTERS="4"
    TOTALS=$((SIZE*KB))
    ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}KB_D${DISTANCE}_P${POINTERS}${LOG}"

    mv *${FILE}*.* ${FILE}

done
