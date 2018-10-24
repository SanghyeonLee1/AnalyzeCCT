#!/bin/bash
###############################
#### Made by Sanghyeon Lee ####
###############################

export LC_NUMERIC="en_US.UTF-8" #set correct locale for printf command

EXPECTED_ARGS=1
if [ ! $# -eq $EXPECTED_ARGS ]
then
    echo "number of arguments not correct!"
    echo "please type path to data file:"
    exit
fi

MATRIX_DIR=$1
SOFTWARE_DIR="/Users/sanghyeon/Investigator/CCT/"
DATA_DIR="$MATRIX_DIR/oscilloscope/"

cat $SOFTWARE_DIR/data/vbb_list.txt | \
while read vbb
do
    echo "Now I'm going to draw vbb= $vbb V"
    MEAS_DIR=$(printf "$DATA_DIR/VBB-%0.1f" ${vbb})
    echo $MEAS_DIR
    cd $MEAS_DIR
    ls -1v C16* > list.txt
    cd $SOFTWARE_DIR/analysis

    root -l -b <<EOF &
    .x analyze.C+g("${MEAS_DIR}/", ${vbb})
    .q
EOF
done

