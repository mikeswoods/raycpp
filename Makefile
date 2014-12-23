

ifdef VERBOSE
	Q =
	E = @true 
else
	Q = @
	E = @echo 
endif

################################################################################

CXX = g++

PROJECT   = raycpp
SRC_DIR   = src
BUILD_DIR = build

################################################################################

CXXFLAGS = -fopenmp -O2 --std=c++11 -Wall -Wextra -Wno-reorder -Wno-unused-parameter
LDFLAGS  = -fopenmp

INCLUDE_DIRS := include
LIBRARY_DIRS :=
LIBRARIES    := GLEW GL GLU glut

################################################################################

CXXFILES := $(shell find $(SRC_DIR) -mindepth 1 -maxdepth 4 -name "*.cpp")
INFILES  := $(CXXFILES)

OBJFILES := $(CXXFILES:src/%.cpp=%)
DEPFILES := $(CXXFILES:src/%.cpp=%)
OFILES := $(OBJFILES:%=$(BUILD_DIR)/%.o)

################################################################################

ifdef DEBUG
	CXXFLAGS := $(CXXFLAGS) -g
endif

CXXFLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
LDFLAGS  += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir))
LDFLAGS  += $(foreach library,$(LIBRARIES),-l$(library))

################################################################################

.PHONY: clean all depend
.SUFFIXES:

$(BUILD_DIR)/%.o: src/%.cpp
	$(E)C++-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(Q)$(CXX) -o $@ -c $< $(CXXFLAGS)

$(PROJECT): $(OFILES)
	$(E)Linking $@
	$(Q)$(CXX) -o $@ $(OFILES) $(LDFLAGS)

clean:
	$(E)Removing files
	$(Q)rm -f $(PROJECT) $(BUILD_DIR)/* Makefile.dep
