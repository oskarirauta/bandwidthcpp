all: world

CXX?=g++
CXXFLAGS?=--std=c++23 -Wall -fPIC
LDFLAGS?=-L/lib -L/usr/lib

INCLUDES+= -I./examples/include

BANDWIDTHCPP_DIR:=.
include Makefile.inc

world: bmtest

objs/bmtest.o: examples/bmtest.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

bmtest: $(BANDWIDTH_OBJS) objs/bmtest.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -L. $(UBUS_LIBS) $^ -o $@;

.PHONY: clean
clean:
	rm -f objs/*.o bmtest
