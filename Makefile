#
#
#

CPP = g++-8
CPPNATIVE = g++
CPP_FLAGS = -std=c++17 -g -O2 -lstdc++fs #Using -O2 to relatively aggressively optimize. Use -g to use with gdb
#gdb requires hacks for now.

.PHONY: all clean
Root_Directory =  ${strip $(CURDIR)}
SCDIR = $(Root_Directory)
SOURCEDIR = $(SCDIR)/src
BINDIR = $(Root_Directory)/bin
OBJECT_DIR = $(SCDIR)/objects

#Creating directories for .o as in https://stackoverflow.com/questions/1950926/create-directories-using-make-file
#MKDIR = mkdir -p
#directories: ${OUT_DIR}
all: testbed_process

default: testbed_process

#${OUT_DIR}:
#	${MKDIR_P} ${OUT_DIR}

#DIRECTORIES = $(sort $(dir $(wildcard $(SOURCEDIR)/*/)))


$(OBJECT_DIR)/%.cpp.o: $(SOURCEDIR)/%.cpp
	mkdir -p $(@D)
	$(CPP) $(CPP_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(OBJECT_DIR)/%.cpp.n.o: $(SOURCEDIR)/%.cpp
	mkdir -p $(@D)
	$(CPPNATIVE) $(CPP_FLAGS) $(INCLUDE_DIRS) -c $< -o $@
	

#Grabs all .cpp in the search client folder
CPP_FILES = $(wildcard $(SOURCEDIR)/*.cpp)
CPPSHORT = $(subst $(SCDIR),,$(CPP_FILES))
OBJS = $(foreach d, $(CPPSHORT), $(OBJECT_DIR)$(d).o)
#%.cpp.o:	%.cpp
#	$(CPP) $(CPP_FLAGS) $(INCLUDE_DIRS) $< -o $@

#OBJ_SC = $(foreach d, $(basename $(CPP_FILES)), $(d).cpp.o)

testbed_process: $(OBJECT_DIR)/testbed.cpp.o $(OBJECT_DIR)/auxiliary.cpp.o $(OBJECT_DIR)/execcmd.cpp.o $(OBJECT_DIR)/getmac.cpp.o $
	mkdir -p $(BINDIR)
	$(CPP) -o $(BINDIR)/testbed_process $^ $(CPP_FLAGS)

EXECUTABLE += $(Root_Directory)/testbed_process

sync_process: $(OBJECT_DIR)/sync.cpp.n.o $(OBJECT_DIR)/auxiliary.cpp.n.o
	$(CPPNATIVE) -o $(BINDIR)/sync_process $^ $(CPP_FLAGS)

#-I marks that headers can be found in a directory
INCLUDE_TEMP += $(SCDIR)/include



#Takes all of include_temp, and puts on -I flag.
INCLUDE_DIRS = $(foreach dir, $(basename $(INCLUDE_TEMP)), -I$(dir))

#EXECUTABLES += $(Root_Directory)/prober $(Root_Directory)/maketable $(Root_Directory)/searcher

clean:
	rm $(OBJS)

export INCLUDE_DIRS 
