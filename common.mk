# Build with clang
CC  := clang
CXX := clang++
NVCC := nvcc -arch sm_20
NVCC_FLAGS := -g -I/home/curtsinger/include -L/home/curtsinger/lib -ljansson

# Default flags
CFLAGS   ?= -g -O2 --std=c11
CXXFLAGS ?= -g -O2 --std=c++11
LDFLAGS  += $(addprefix -l,$(LIBS))

# Default source and object files
SRCS    ?= $(wildcard *.cc) $(wildcard *.cpp) $(wildcard *.c) $(wildcard *.cu)
OBJS    ?= $(addprefix obj/,$(patsubst %.cu,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(SRCS))))))

# Targets to build recirsively into $(DIRS)
RECURSIVE_TARGETS  ?= all clean test

# Targets separated by type
SHARED_LIB_TARGETS := $(filter %.so, $(TARGETS))
STATIC_LIB_TARGETS := $(filter %.a, $(TARGETS))
OTHER_TARGETS      := $(filter-out %.so, $(filter-out %.a, $(TARGETS)))

# If not set, the build path is just the current directory name
MAKEPATH ?= `basename $(PWD)`

# Log the build path in gray, following by a log message in bold green
LOG_PREFIX := "$(shell tput setaf 7)[$(MAKEPATH)]$(shell tput sgr0)$(shell tput setaf 2)"
LOG_SUFFIX := "$(shell tput sgr0)"

# Build all targets by default
all:: $(TARGETS)

# Clean up after a build
clean::
	@for t in $(TARGETS); do \
	echo $(LOG_PREFIX) Cleaning $$t $(LOG_SUFFIX); \
	done
	@rm -rf $(TARGETS) obj

test::

# Prevent errors if files named all, clean, bench, or test exist
.PHONY: all clean test

# Compile a C++ source file (and generate its dependency rules)
obj/%.o: %.cc $(PREREQS)
	@echo $(LOG_PREFIX) Compiling $< $(LOG_SUFFIX)
	@mkdir -p obj
	@$(CXX) $(CXXFLAGS) -MMD -MP -o $@ -c $<

# Compile a C++ source file (and generate its dependency rules)
obj/%.o: %.cpp $(PREREQS)
	@echo $(LOG_PREFIX) Compiling $< $(LOG_SUFFIX)
	@mkdir -p obj
	@$(CXX) $(CXXFLAGS) -MMD -MP -o $@ -c $<

# Compile a C source file (and generate its dependency rules)
obj/%.o: %.c $(PREREQS)
	@echo $(LOG_PREFIX) Compiling $< $(LOG_SUFFIX)
	@mkdir -p obj
	@$(CC) $(CFLAGS) -MMD -MP -o $@ -c $<

# Compile a Cuda source file (and generate its dependency rules)
obj/%.o: %.cu $(PREREQS)
	@echo $(LOG_PREFIX) Compiling $< $(LOG_SUFFIX)
	@mkdir -p obj
	@$(NVCC) $(NVCC_FLAGS) -o $@ -c $<

# Link a shared library
$(SHARED_LIB_TARGETS): $(OBJS)
	@echo $(LOG_PREFIX) Linking $@ $(LOG_SUFFIX)
	@$(CXX) -shared $(LDFLAGS) -o $@ $^

$(STATIC_LIB_TARGETS): $(OBJS)
	@echo $(LOG_PREFIX) Linking $@ $(LOG_SUFFIX)
	@ar rs $@ $^

# Link binary targets
$(OTHER_TARGETS): $(OBJS)
	@echo $(LOG_PREFIX) Linking $@ $(LOG_SUFFIX)
	@$(CXX) $(LDFLAGS) -o $@ $^

# Include dependency rules for all objects
-include $(OBJS:.o=.d)

# Build any recursive targets in subdirectories
$(RECURSIVE_TARGETS)::
	@for dir in $(DIRS); do \
	$(MAKE) -C $$dir --no-print-directory $@ MAKEPATH="$(MAKEPATH)/$$dir"; \
	done
