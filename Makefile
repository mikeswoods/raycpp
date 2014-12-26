

ifdef VERBOSE
	Q =
	E = @true 
else
	Q = @
	E = @echo 
endif

################################################################################

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
CXX = g++-4.9
else
CXX = g++
endif

PROJECT   = raycpp
SRC_DIR   = src
BUILD_DIR = build

################################################################################

CXXFLAGS = \
    -fdiagnostics-color=always \
	-O2 \
	--std=c++11 \
	-Wall \
	-Wextra \
	-Wno-reorder \
	-Wno-unused-parameter \
	-Wno-unused-function

ifeq ($(UNAME), Darwin)
	CXXFLAGS += -fopenmp
endif

################################################################################

ifeq ($(UNAME), Darwin)
LDFLAGS = -fopenmp -lGLEW -framework GLUT -framework OpenGL -framework Cocoa
else
LDFLAGS = -fopenmp -lGLEW -lGL -lGLU -lglut
endif

################################################################################

INCLUDE_DIRS := include

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
