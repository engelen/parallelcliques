# Makefile for SDDM
include Makefile.config

## Main application file
MAIN = main
DEPH = $(DIR_GRAPH)/Graph.h $(DIR_CLIQUEFINDER)/CliqueFinder.h $(DIR_PROCESS)/Process.h $(DIR_PROCESS)/ProcessMaster.h $(DIR_PROCESS)/ProcessSlave.h $(DIR_PROGRESSMONITOR)/ProgressMonitor.h
DEPCPP = $(DIR_GRAPH)/Graph.cpp $(DIR_CLIQUEFINDER)/CliqueFinder.cpp $(DIR_PROCESS)/Process.cpp $(DIR_PROCESS)/ProcessMaster.cpp $(DIR_PROCESS)/ProcessSlave.cpp $(DIR_PROGRESSMONITOR)/ProgressMonitor.cpp
CC = mpic++

all: $(MAIN)

# COMPILE
$(MAIN): $(MAIN).cpp $(DEPH) $(DEPCPP) 
	$(CC) $(CXXFLAGS) $(CXXOPENMP) -o $(MAIN) $(MAIN).cpp $(DEPCPP) $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o  $(MAIN)  $(MAIN).exe
	rm -rf Debug Release