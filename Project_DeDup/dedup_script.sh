#!/bin/bash
CSV=".csv"
LOG=".log"
SIZE="4"
KB="1024"
MB=$((KB*KB))
TOTALS=$((SIZE*MB))
DISTANCE="0"
POINTERS="0"
rm -rf runs.csv

for FILE in $( ls | grep ".csv" | cut -f 1 -d . ); do
    rm -rf ${FILE}
	mkdir ${FILE}

    /usr/bin/time -f "%E, %M" ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}MB_D${DISTANCE}_P${POINTERS}${LOG}" 

	POINTERS="128"
    /usr/bin/time -f "%E, %M" ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}MB_D${DISTANCE}_P${POINTERS}${LOG}" 


    SIZE="8"
    TOTALS=$((SIZE*MB))
    POINTERS="0"
	/usr/bin/time -f "%E, %M" ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}MB_D${DISTANCE}_P${POINTERS}${LOG}" 

    POINTERS="128"
    /usr/bin/time -f "%E, %M" ./dedup ${FILE}${CSV} ${TOTALS} ${DISTANCE} ${POINTERS} < params > "${FILE}_${SIZE}MB_D${DISTANCE}_P${POINTERS}${LOG}"  

    mv *${FILE}*.* ${FILE}

done
