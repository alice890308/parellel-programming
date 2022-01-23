#!/bin/bash
make clean
make

srun -c6 -n2 ./hw2b experiment.png 10000 -0.29899250664589705 -0.2772002993319328 -0.6327591639095336 -0.6433840614725646 7680 4320
make clean

rm experiment.png