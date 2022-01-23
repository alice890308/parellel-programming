#!/bin/bash
make clean
make

srun -c12 -n1 ./hw2a experiment.png 10000 -0.29899250664589705 -0.2772002993319328 -0.6327591639095336 -0.6433840614725646 7680 4320
make clean

rm experiment.png