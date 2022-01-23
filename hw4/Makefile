CC = mpicc
CXX = mpicxx
CXXFLAGS = -O3 -lm
CFLAGS = -O3 -lm
TARGETS = main

.PHONY: all
all: 
	mpicxx -std=c++11 -O3 -lm -o mapreduce main.cc scheduler.cc  scheduler.h worker.cc worker.h mapper.cc mapper.h reducer.cc reducer.h -lboost_filesystem

address:
	mpicxx -std=c++11 -fsanitize=address -g -O3 -lm -o mapreduce main.cc scheduler.cc  scheduler.h worker.cc worker.h mapper.cc mapper.h reducer.cc reducer.h -lboost_filesystem

.PHONY: clean
clean:
	rm -f $(TARGETS) *.h.gch mapreduce

cleanL:
	rm -rf ./interFile log.txt ./TEST*

cleanC:
	rm check.txt
