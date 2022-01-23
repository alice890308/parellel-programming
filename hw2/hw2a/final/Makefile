CC = gcc
CXX = g++
LDLIBS = -lpng
CFLAGS = -lm -O3 -ffast-math
hw2a: CFLAGS += -pthread
hw2a_v1: CFLAGS += -pthread
hw2b: CC = mpicc
hw2b: CXX = mpicxx
hw2b: CFLAGS += -fopenmp
CXXFLAGS = $(CFLAGS)
TARGETS = hw2a

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
