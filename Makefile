# File:      Makefile
# Date:      April 2013
# Course:    POS - Advanced Operating Systems
# Project:   Shell
# Author:    Radim Loskot - xlosko01
# Compiler:  implicit (g++)

# Pouziti:
#  - make               compile release version
#  - make run           run program
#  - make pack          packs all required files to compile this project    
#  - make clean         clean temp compilers files    

# output project and package filename
SRC_DIR=src
OBJ_DIR=obj
TARGET=shell
PACKAGE_NAME=xlosko01
PACKAGE_FILES=Makefile src/shell.cpp src/PThread.cpp src/PThread.h src/ReadPThread.cpp src/ReadPThread.h src/ExecutePThread.cpp src/ExecutePThread.h src/UniqueIDGenerator.cpp src/UniqueIDGenerator.h src/ShellService.cpp src/ShellService.h src/RegExp.cpp src/RegExp.h

# C++ compiler and flags
CXX=g++
CXXFLAGS=$(CXXOPT) -Wall -pedantic -W -ansi -std=c++98
LIBS=-lpthread #-lpthreads

# Project files
OBJ_FILES=shell.o PThread.o ReadPThread.o ExecutePThread.o UniqueIDGenerator.o ShellService.o RegExp.o
SRC_FILES=shell.cpp PThread.cpp ReadPThread.cpp ExecutePThread.cpp UniqueIDGenerator.cpp ShellService.cpp RegExp.cpp

# Substitute the path
SRC=$(patsubst %,$(SRC_DIR)/%,$(SRC_FILES))

OBJ=$(patsubst %,$(OBJ_DIR)/%,$(OBJ_FILES))

# Universal rule
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# START RULE
all: | $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Linking of modules into release program
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean pack run debug release

pack:
	zip $(PACKAGE_NAME).zip $(PACKAGE_FILES)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(TARGET)

debug:
	make -B all CXXOPT=-g3
	
release:
	make -B all CXXOPT=-O3

run:
	./$(TARGET)