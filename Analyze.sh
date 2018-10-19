#!/bin/bash

root -l -b <<EOF &
.x test6.C+("../data/mm36/VBB_0V")
.x test6.C+("../data/mm36/VBB_-1V")
.q
EOF
