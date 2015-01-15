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
CXX = ccache g++-4.9
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
	-pedantic \
	-Wno-unused-parameter \
	-Wno-unused-function \
	-Wno-variadic-macros \
	-ggdb

ifeq ($(UNAME), Darwin)
	CXXFLAGS += -fopenmp -fdiagnostics-color=always `pkg-config --cflags glfw3`
else
	ifeq ($(UNAME), Linux)
		CXXFLAGS += -fopenmp `pkg-config --cflags glfw3`
	endif
endif

################################################################################

ifeq ($(UNAME), Darwin)
LDFLAGS = -L/usr/X11/lib -fopenmp -lGLEW `pkg-config --static --libs glfw3` -lX11
else
LDFLAGS = -L/usr/X11/lib -fopenmp -lGLEW `pkg-config --static --libs glfw3` -lX11
endif

################################################################################

INCLUDE_DIRS := include /opt/X11/include

################################################################################

#CXXFILES := $(shell find $(SRC_DIR) -mindepth 1 -maxdepth 4 -name "*.cpp")
CXXFILES := $(shell find $(SRC_DIR) -mindepth 1 -maxdepth 1 -name "*.cpp")
INFILES  := $(CXXFILES)

OBJFILES := $(CXXFILES:src/%.cpp=%)
DEPFILES := $(CXXFILES:src/%.cpp=%)
OFILES := $(OBJFILES:%=$(BUILD_DIR)/%.o)

################################################################################

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
	$(Q)$(CXX) $(CXXFLAGS) -o $@ $(OFILES) $(LDFLAGS)

test_loader:	build/ObjReader.o\
				build/Mesh.o \
				build/Geometry.o \
				build/Tri.o \
				build/KDTree.o \
				build/Ray.o \
				build/AABB.o \
				build/R3.o \
				build/Intersection.o \
				build/Utils.o \
				build/Face.o \
				build/Color.o \
				src/test/test_loader.cpp
	$(CXX) -o $@ $(CXXFLAGS) $^

clean:
	$(E)Removing files
	$(Q)rm -f $(PROJECT) $(BUILD_DIR)/* Makefile.dep test_loader
