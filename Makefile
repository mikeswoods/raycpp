WITH_OMP=1

ifdef VERBOSE
	Q =
	E = @true 
else
	Q = @
	E = @echo 
endif

ifdef WITH_OMP
	CXXFLAGS += -DENABLE_OPENMP
endif

################################################################################

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
CXX = g++-4.9
else
	ifeq ($(UNAME), Linux)
		CXX = g++
	else
		CXX = g++
	endif
endif

PROJECT   = raycpp
SRC_DIR   = src
BUILD_DIR = build

################################################################################

CXXFLAGS += \
	-O3 \
	--std=c++11 \
	-Wall \
	-Wextra \
	-Wno-reorder \
	-Wno-unused-parameter \
	-Wno-unused-function 

ifeq ($(UNAME), Darwin)
	CXXFLAGS += -fopenmp -fdiagnostics-color=always `pkg-config --cflags glfw3`
else
	ifeq ($(UNAME), Linux)
		CXXFLAGS += -fopenmp `pkg-config --cflags glfw3`
	endif
endif

################################################################################

ifeq ($(UNAME), Darwin)
LDFLAGS = -fopenmp -lGLEW `pkg-config --static --libs glfw3`
else
LDFLAGS = -fopenmp -lGLEW `pkg-config --static --libs glfw3`
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
