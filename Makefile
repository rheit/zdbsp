CC = gcc
CXX = g++

CFLAGS = -Wall -Izlib -pipe -ffast-math -MMD

# Optimization flags
CFLAGS += -O3 -fomit-frame-pointer -DNDEBUG

# Unoptimization flags
#CFLAGS += -g -D_DEBUG

# Processor features flags
CFLAGS += -mtune=i686
#CFLAGS += -march=k8

LDFLAGS =
RM = rm -f FILE
ZLIBDIR = zlib/

ifeq (Windows_NT,$(OS))
  EXE = zdbsp.exe
  LDFLAGS += -luser32 -lgdi32 -static-libstdc++ -static-libgcc
  ifneq (msys,$(OSTYPE))
    RM = del /q /f FILE 2>nul
    ZLIBDIR = "zlib\"
  endif
else
  EXE = zdbsp
  CFLAGS += -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -DNO_MAP_VIEWER=1
endif

# To generate profiling information for gprof, pass gprof=1 to make.
ifeq ($(gprof),1)
  CFLAGS += -g -fno-omit-frame-pointer -pg
  LDFLAGS += -g -pg
endif

# To strip debugging symbols, pass strip=1 to make.
ifeq ($(strip),1)
  LDFLAGS += -s
endif

# To compile without support for backpatching ClassifyLine calls, pass nobackpatch=1 to make.
ifeq ($(nobackpatch),1)
  CFLAGS += -DDISABLE_BACKPATCH
endif

# To use SSE2 math for everything, pass sse=1 to make.
ifeq ($(sse),1)
  CFLAGS += -msse2 -mfpmath=sse
endif

OBJS = main.o getopt.o getopt1.o blockmapbuilder.o processor.o processor_udmf.o sc_man.o view.o wad.o \
	nodebuild.o nodebuild_events.o nodebuild_extract.o nodebuild_gl.o \
	nodebuild_utility.o nodebuild_classify_nosse2.o \
	zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/deflate.o zlib/trees.o \
	zlib/zutil.o

# To compile without any SSE support, pass nosse=1 to make.
ifeq ($(nosse),1)
  CFLAGS += -DDISABLE_SSE
else
  OBJS += nodebuild_classify_sse1.o nodebuild_classify_sse2.o
endif

CXXFLAGS = $(CFLAGS)

ifeq (Windows_NT,$(OS))
  OBJS += resource.o
endif

all: $(EXE)

profile:
	$(MAKE) cleanall
	$(MAKE) all CFLAGS="$(CFLAGS) -fprofile-generate" LDFLAGS="$(LDFLAGS) -lgcov"
	@echo "Process a few maps, then rebuild with make profile-use"

profile-use:
	$(MAKE) clean
	$(MAKE) all CXXFLAGS="$(CXXFLAGS) -fprofile-use"

$(EXE): $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(LDFLAGS)

nodebuild_classify_sse2.o: nodebuild_classify_sse2.cpp nodebuild.h
	$(CXX) $(CXXFLAGS) -msse -msse2 -march=i686 -mfpmath=sse -c -o $@ $<

nodebuild_classify_sse1.o: nodebuild_classify_sse1.cpp nodebuild.h
	$(CXX) $(CXXFLAGS) -msse -march=i686 -mfpmath=sse -c -o $@ $<

resource.o: resource.rc
	windres -o $@ -i $<

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
