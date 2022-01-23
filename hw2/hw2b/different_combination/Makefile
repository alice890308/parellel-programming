CC = mpicc
CXX = mpicxx
LDLIBS = -lpng
CFLAGS = -lm -O3 -pthread
hw2b: -cxx=clang++
hw2b: -cc=clang
hw2b: CFLAGS += -fopenmp
CXXFLAGS = $(CFLAGS)
TARGETS = hw2b

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
