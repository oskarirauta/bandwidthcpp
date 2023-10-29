all: world

CXX?=g++
CXXFLAGS?=--std=c++23 -Wall -fPIC
LDFLAGS?=-L/lib -L/usr/lib

INCLUDES+= -I./examples/include -I.

BANDWIDTHCPP_DIR:=.
include Makefile.inc
include cmdparser/Makefile.inc

world: bmtest

objs/bmtest.o: examples/bmtest.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

bmtest: $(BANDWIDTH_OBJS) $(CMDPARSER_OBJS) objs/bmtest.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

.PHONY: clean
clean:
	rm -f objs/*.o bmtest
