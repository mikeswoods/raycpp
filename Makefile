
VERBOSE=1
ifdef VERBOSE
		Q =
		E = @true 
else
		Q = @
		E = @echo 
endif

################################################################################

CXX = g++

PROJECT   = upray
SRC_DIR   = "src"
BUILD_DIR = "build"

################################################################################

CXXFLAGS = -MM --std=c++0x -Wall -Wextra -Wno-reorder -Wno-unused-parameter
LDFLAGS  =

INCLUDE_DIRS := include
LIBRARY_DIRS :=
LIBRARIES    := GLEW GL GLU glut

################################################################################

CXXFILES := $(shell find $(SRC_DIR) -mindepth 1 -maxdepth 4 -name "*.cpp")
INFILES  := $(CFILES) $(CXXFILES)

OBJFILES := $(CXXFILES:src/%.cpp=%)
DEPFILES := $(CXXFILES:src/%.cpp=%.d)
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

Makefile.dep: $(CFILES) $(CXXFILES)
		$(E)Depend
		$(Q)for i in $(^); do $(CXX) $(CXXFLAGS) -MM "$${i}" -MT $(BUILD_DIR)/`basename $${i%.*}`.o; done > $@

$(PROJECT): Makefile.dep $(OFILES)
	$(E)Linking $@
	$(Q)$(CXX) -o $@ $(OFILES) -fopenmp $(LDFLAGS)

clean:
	$(E)Removing files
	$(Q)rm -f $(PROJECT) $(BUILD_DIR)/* Makefile.dep
