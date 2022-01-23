CC = gcc
CXX = g++
LDLIBS = -lpng
CFLAGS = -lm -O3
hw3-1: CFLAGS += -fopenmp
tmp: CFLAGS += -fopenmp
CXXFLAGS = $(CFLAGS)
TARGETS = hw3-1 tmp

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
