### Determine compiler invocation ###

ifdef PE_ENV

# Cray-specific invocations
CC = cc
CXX = CC
MPICC = $(CC)
MPICXX = $(CXX)
FC = ftn
F77 = $(FC)
F90 = $(FC)
MPIF77 = $(FC)
MPIF90 = $(FC)

else

ifneq ($(filter default undefined,$(origin FC)),)
# default to GNU
FC := gfortran
endif
F77 ?= $(FC)
F90 ?= $(FC)

# MPI C/C++ Compilers
ifndef MPICC
ifeq ($(shell which mpiicc > /dev/null 2>&1; echo $$?),0)
MPICC := mpiicc
else ifeq ($(shell which mpicc > /dev/null 2>&1; echo $$?),0)
MPICC := mpicc
endif
endif
# Only detect toolchain if MPICC is set, otherwise defer error to rule which invokes compiler
ifdef MPICC
MPICC_VERSION := $(shell $(MPICC) --version 2> /dev/null || $(MPICC) -qversion 2> /dev/null)
else
MPICC = $(error Could not detect MPI C compiler in PATH - failed to make target $@)
endif

ifndef MPICXX
ifeq ($(shell which mpiicpc > /dev/null 2>&1; echo $$?),0)
MPICXX := mpiicpc
else ifeq ($(shell which mpicxx > /dev/null 2>&1; echo $$?),0)
MPICXX := mpicxx
endif
endif
MPICXX ?= $(error Could not detect MPI C++ compiler in PATH - failed to make target $@)

# MPI Fortran Compilers
ifndef MPIF90
ifeq ($(shell which mpiifort > /dev/null 2>&1; echo $$?),0)
MPIF90 := mpiifort
else ifeq ($(shell which mpifc > /dev/null 2>&1; echo $$?),0)
MPIF90 := mpifc
else ifeq ($(shell which mpifort > /dev/null 2>&1; echo $$?),0)
MPIF90 := mpifort
else ifeq ($(shell which mpif90 > /dev/null 2>&1; echo $$?),0)
MPIF90 := mpif90
endif
endif

# Only detect toolchain if MPIF90 is set, otherwise defer error to rule which invokes compiler
ifdef MPIF90
MPIF90_VERSION := $(shell $(MPIF90) --version 2> /dev/null || $(MPIF90) -qversion 2> /dev/null)
else
MPIF90 = $(error Could not detect MPI Fortran compiler in PATH - failed to make target $@)
endif

ifndef MPIF77
ifeq ($(shell which mpif77 > /dev/null 2>&1; echo $$?),0)
MPIF77 := mpif77
else
MPIF77 = $(MPIF90)
endif
endif

MPIFC ?= $(MPIF90)

endif

### Recommended compiler flags ###

# Flags for compiler inlining: MAP works whether inlining is on or off,
# but you'll typically see more intuitive stacks with it turned off.
# The major compilers are discussed here:
#
# Intel: -g -fno-inline -no-ip -no-ipo -fno-omit-frame-pointer -O3 is
# recommended. At O3 the compiler doesn't produce enough unwind info even
# with -debug inline-debug-info set.
#
# PGI: -g -O3 -Meh_frame -Mframe -Mnoautoinline is recommended. Other settings
# dont produce enough unwind information for inlined functions otherwise. This
# adds some performance penalty - around 8% is typical.
#
# The PGI C runtime static library contains an undefined reference to
# __kmpc_fork_call, which will cause compilation to fail when linking
# allinea-profiler.ld. Add --undefined __wrap___kmpc_fork_call to your link line
# before linking to the MAP sampler to resolve this.
#
# GNU: -g -O3 -fno-inline is recommended. You might be lucky without -fno-inline,
# as it should produce enough information to unwind those calls. You will see
# my_function [inlined] in the MAP stack for functions that were inline.
# -fno-inline-functions appears with newer gnu compilers, just to confuse

# Common OpenMP flags for supported compilers
# -fopenmp for gnu
# -openmp  for intel
# -mp      for pgi
# -qsmp=omp:noopt for IBM
# -homp    for cray (compiler)

# Common pthread flags for supported compilers
# -pthread for GNU
# -lpthread for other compilers

INTEL_MAP_CFLAGS := -g -fno-inline -no-ip -no-ipo -fno-omit-frame-pointer -O3
INTEL_OPENMP_CFLAG := -qopenmp
INTEL_MAP_FCFLAGS := $(INTEL_MAP_CFLAGS)
INTEL_OPENMP_FCFLAG := $(INTEL_OPENMP_CFLAG)
INTEL_PTHREAD_CFLAG := -lpthread

PGI_MAP_CFLAGS := -g -Meh_frame -Mframe -O3 -Wl,--undefined=__wrap___kmpc_fork_call -Mnoautoinline
PGI_OPENMP_CFLAG := -mp
PGI_MAP_FCFLAGS := $(filter-out -Meh_frame, $(PGI_MAP_CFLAGS))
PGI_OPENMP_FCFLAG := $(PGI_OPENMP_CFLAG)
PGI_PTHREAD_CFLAG := -lpthread

IBM_MAP_CFLAGS := -g -O3 -qnoinline
IBM_OPENMP_CFLAG := -qsmp=omp:noopt
IBM_MAP_FCFLAGS := $(IBM_MAP_CFLAGS)
IBM_OPENMP_FCFLAG := $(IBM_OPENMP_CFLAG) -qsmp=omp:noopt -qnohot -lxlf90 -lxlsmp -lxlfmath
IBM_PTHREAD_CFLAG := -lpthread

CRAY_MAP_CFLAGS := -g -O3 -hipa0
CRAY_OPENMP_CFLAG := -homp
CRAY_MAP_FCFLAGS := $(CRAY_MAP_CFLAGS)
CRAY_OPENMP_FCFLAG := $(CRAY_OPENMP_CFLAG)
CRAY_PTHREAD_CFLAG := -lpthread

GNU_MAP_CFLAGS := -g -O3 -fno-inline
GNU_OPENMP_CFLAG := -fopenmp
GNU_MAP_FCFLAGS := $(GNU_MAP_CFLAGS)
GNU_OPENMP_FCFLAG := $(GNU_OPENMP_CFLAG)
GNU_PTHREAD_CFLAG := -pthread

### Toolchain detection ###

define get_compiler_toolchain
$(if $(or $(findstring icc,$(1)),$(findstring ifort,$(1)),$(findstring Intel,$(1)),$(findstring INTEL,$(PE_ENV))),
	INTEL,
$(if $(or $(findstring pgcc,$(1)),$(findstring pgfortran,$(1)),$(findstring PGI,$(1)), $(findstring PGI,$(PE_ENV))),
	PGI,
$(if $(or $(findstring xlc,$(1)),$(findstring xlf,$(1)),$(findstring IBM,$(1)),$(findstring IBM,$(PE_ENV))),
	IBM,
$(if $(findstring CRAY,$(PE_ENV)),
	CRAY,
	GNU))))
endef

CC_TOOLCHAIN := $(strip $(call get_compiler_toolchain,$(CC)))
MPICC_TOOLCHAIN := $(strip $(call get_compiler_toolchain,$(MPICC_VERSION)))
FC_TOOLCHAIN := $(strip $(call get_compiler_toolchain,$(FC)))
MPIF90_TOOLCHAIN := $(strip $(call get_compiler_toolchain,$(MPIF90_VERSION)))

### Compiler flags for toolchain (allow overrides) ###

MAP_CFLAGS ?= $($(CC_TOOLCHAIN)_MAP_CFLAGS)
MAP_FCFLAGS ?= $($(FC_TOOLCHAIN)_MAP_FCFLAGS)
OPENMP_CFLAG ?= $($(CC_TOOLCHAIN)_OPENMP_CFLAG)
OPENMP_FCFLAG ?= $($(FC_TOOLCHAIN)_OPENMP_FCFLAG)
PTHREAD_CFLAG ?= $($(CC_TOOLCHAIN)_PTHREAD_CFLAG)
PTHREAD_FCFLAG ?= $($(FC_TOOLCHAIN)_PTHREAD_FCFLAG)
MPI_MAP_CFLAGS ?= $($(MPICC_TOOLCHAIN)_MAP_CFLAGS)
MPI_MAP_FCFLAGS ?= $($(MPIF90_TOOLCHAIN)_MAP_FCFLAGS)
MPI_OPENMP_CFLAG ?= $($(MPICC_TOOLCHAIN)_OPENMP_CFLAG)
MPI_OPENMP_FCFLAG ?= $($(MPIF90_TOOLCHAIN)_OPENMP_FCFLAG)
MPI_PTHREAD_CFLAG ?= $($(MPICC_TOOLCHAIN)_PTHREAD_CFLAG)
MPI_PTHREAD_FCFLAG ?= $($(MPIF90_TOOLCHAIN)_PTHREAD_FCFLAG)
