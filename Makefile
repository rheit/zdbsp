CFLAGS = -Wall -fomit-frame-pointer -Izlib -pipe -ffast-math -MMD

# Optimization flags
CFLAGS += -O3 -fomit-frame-pointer -DNDEBUG

# Unoptimization flags
#CFLAGS += -g -D_DEBUG

# Processor features flags
CFLAGS += -mtune=i686
#CFLAGS += -march=k8 -mfpmath=sse

LDFLAGS =
RM = rm -f FILE
ZLIBDIR = zlib/

ifeq (Windows_NT,$(OS))
  EXE = zdbsp.exe
  LDFLAGS += -luser32 -lgdi32
  ifneq (msys,$(OSTYPE))
    RM = del /q /f FILE 2>nul
    ZLIBDIR = "zlib\"
  endif
else
  EXE = zdbsp
  CFLAGS += -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -DNO_MAP_VIEWER=1
endif

# To generate profiling information for gprof, pass gprof=1 to make.
ifneq ($(gprof),)
  CFLAGS += -g -fno-omit-frame-pointer -pg
  LDFLAGS += -g -pg
endif

# To strip debugging symbols, pass strip=1 to make.
ifneq ($(strip),)
  LDFLAGS += -s
endif

CC = gcc
CXX = g++

CXXFLAGS = $(CFLAGS)

OBJS = main.o getopt.o getopt1.o blockmapbuilder.o processor.o view.o wad.o \
	nodebuild.o nodebuild_events.o nodebuild_extract.o nodebuild_gl.o \
	nodebuild_utility.o \
	zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/deflate.o zlib/trees.o \
	zlib/zutil.o

all: $(EXE)

profile:
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(CFLAGS) -fprofile-generate" LDFLAGS="$(LDFLAGS) -lgcov"
	@echo "Process a few maps, then rebuild with make profile-use"

profile-use:
	$(MAKE) clean
	$(MAKE) all CXXFLAGS="$(CXXFLAGS) -fprofile-use"

$(EXE): $(OBJS)
	$(CCDV) $(CXX) -o $(EXE) $(OBJS) $(LDFLAGS)

.PHONY: clean

clean:
	$(subst FILE,$(EXE),$(RM))
	$(subst FILE,*.o,$(RM))
	$(subst FILE,*.d,$(RM))
	$(subst FILE,$(ZLIBDIR)*.o,$(RM))
	$(subst FILE,$(ZLIBDIR)*.d,$(RM))

cleanprof:
	$(subst FILE,*.gc*,$(RM))
	$(subst FILE,$(ZLIBDIR)*.gc*,$(RM))
	
cleanall: clean cleanprof

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:%.o=%.d)
endif
