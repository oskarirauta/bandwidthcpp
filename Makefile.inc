BANDWIDTHCPP_DIR?=bandwidth
INCLUDES += -I./$(BANDWIDTHCPP_DIR)/include

BANDWIDTH_OBJS:= \
	objs/bw_interface.o \
	objs/bw_monitor.o

objs/bw_interface.o: $(BANDWIDTHCPP_DIR)/src/bw_interface.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/bw_monitor.o: $(BANDWIDTHCPP_DIR)/src/bw_monitor.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;
