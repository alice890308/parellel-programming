srun -N2 -c12 ./a.out TEST02 7 2 ./testcases/02.word 2 ./testcases/02.loc TEST02 > log.txt

srun -N3 -c10 ./a.out TEST03 10 2 ./testcases/03.word 5 ./testcases/03.loc TEST03 > log.txt

srun -N3 -c12 ./a.out TEST04 10 3 ./testcases/04.word 5 ./testcases/04.loc TEST04 > log.txt

srun -N3 -c12 ./a.out TEST05 10 1 ./testcases/05.word 2 ./testcases/05.loc TEST05 > log.txt

srun -N4 -c10 ./a.out TEST06 12 3 ./testcases/06.word 3 ./testcases/06.loc TEST06 > log.txt

srun -N4 -c12 ./a.out TEST07 13 3 ./testcases/07.word 4 ./testcases/07.loc TEST07 > log.txt

[wrong testcase]srun -N4 -c12 ./a.out TEST08 12 3 ./testcases/08.word 6 ./testcases/08.loc TEST08 > log.txt

srun -N4 -c12 ./a.out TEST09 7 3 ./testcases/09.word 5 ./testcases/09.loc TEST09 > log.txt

srun -N4 -c12 ./a.out TEST10 12 5 ./testcases/10.word 10 ./testcases/10.loc TEST10 > log.txt

srun -N4 -c12 ./a.out TESTlarge 12 5 ./large.word 10 ./large.loc TESTlarge > log.txt