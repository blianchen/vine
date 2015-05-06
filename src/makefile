
##########################
# Supported OSes:
#
#OS         = AIX
#OS         = CYGWIN
#OS         = DARWIN
#OS         = FREEBSD
#OS         = HPUX
#OS         = HPUX_64
#OS         = IRIX
#OS         = IRIX_64
#OS         = LINUX
#OS         = NETBSD
#OS         = OPENBSD
#OS         = OSF1
#OS         = SOLARIS
#OS         = SOLARIS_64

# Please see the "Other possible defines" section below for
# possible compilation options.
##########################

CC          = gcc
#AR          = ar
LD          = ld
#RANLIB      = ranlib
#LN          = ln

SHELL       = /bin/sh
ECHO        = /bin/echo

BUILD       = DBG
TARGETDIR   = ../bin

DEFINES     = -D$(OS)
CFLAGS      =

# set support database,  HAVE_HIREDIS/HAVE_LIBPQ
HAVE_HIREDIS = true
HAVE_LIBPQ = true

# set dir
DIRS := . st luaapi db 


##########################
# Platform section.
# Possible targets:

PLAT := none

PLATS := aix freebsd linux solaris

#
# Platform specifics
#

ifeq ($(OS), LINUX)
EXTRA_OBJS  = $(TARGETDIR)/st/md.o
LDFLAGS     = -lc
OTHER_FLAGS = -Wall
ifeq ($(shell test -f /usr/include/sys/epoll.h && echo yes), yes)
DEFINES     += -DMD_HAVE_EPOLL
endif
endif


ifeq ($(OS), AIX)
AIX_VERSION = $(shell uname -v).$(shell uname -r)
CC          = xlC
ifeq ($(BUILD), OPT)
OTHER_FLAGS = -w
endif
ifneq ($(filter-out 4.1 4.2, $(AIX_VERSION)),)
DEFINES     += -DMD_HAVE_SOCKLEN_T
endif
endif

ifeq ($(OS), CYGWIN)
CC          = gcc
LD          = gcc
DSO_SUFFIX  = dll
SLIBRARY    = $(TARGETDIR)/libst.dll.a
DLIBRARY    = $(TARGETDIR)/libst.dll
DEF_FILE    = $(TARGETDIR)/libst.def
LDFLAGS     = libst.def -shared --enable-auto-image-base -Wl,--output-def,$(DEF_FILE),--out-implib,$(SLIBRARY)
OTHER_FLAGS = -Wall
endif

ifeq ($(OS), DARWIN)
LD          = cc
DSO_SUFFIX  = dylib
RELEASE     = $(shell uname -r | cut -d. -f1)
PPC         = $(shell test $(RELEASE) -le 9 && echo yes)
INTEL       = $(shell test $(RELEASE) -ge 9 && echo yes)
ifeq ($(PPC), yes)
CFLAGS      += -arch ppc
LDFLAGS     += -arch ppc
endif
ifeq ($(INTEL), yes)
CFLAGS      += -arch i386 -arch x86_64
LDFLAGS     += -arch i386 -arch x86_64
endif
LDFLAGS     += -dynamiclib -install_name /sw/lib/libst.$(MAJOR).$(DSO_SUFFIX) -compatibility_version $(MAJOR) -current_version $(VERSION)
OTHER_FLAGS = -Wall
endif

ifeq ($(OS), FREEBSD)
LDFLAGS     = -shared -soname=$(SONAME) -lc
OTHER_FLAGS = -Wall
ifeq ($(shell test -f /usr/include/sys/event.h && echo yes), yes)
DEFINES     += -DMD_HAVE_KQUEUE
endif
endif

ifeq (HPUX, $(findstring HPUX, $(OS)))
ifeq ($(OS), HPUX_64)
DEFINES     = -DHPUX
CFLAGS      = -Ae +DD64 +Z
else
CFLAGS      = -Ae +DAportable +Z
endif
RANLIB      = true
LDFLAGS     = -b
DSO_SUFFIX  = sl
endif

ifeq (IRIX, $(findstring IRIX, $(OS)))
ifeq ($(OS), IRIX_64)
DEFINES     = -DIRIX
ABIFLAG     = -64
else
ABIFLAG     = -n32
endif
RANLIB      = true
CFLAGS      = $(ABIFLAG) -mips3
LDFLAGS     = $(ABIFLAG) -shared
OTHER_FLAGS = -fullwarn
endif

ifeq ($(OS), NETBSD) 
LDFLAGS     = -shared -soname=$(SONAME) -lc
OTHER_FLAGS = -Wall
endif

ifeq ($(OS), OPENBSD)
LDFLAGS     = -shared -soname=$(SONAME) -lc
OTHER_FLAGS = -Wall
ifeq ($(shell test -f /usr/include/sys/event.h && echo yes), yes)
DEFINES     += -DMD_HAVE_KQUEUE
endif
endif

ifeq ($(OS), OSF1)
RANLIB      = true
LDFLAGS     = -shared -all -expect_unresolved "*"
endif

ifeq (SOLARIS, $(findstring SOLARIS, $(OS)))
CC          = gcc
LD          = gcc
RANLIB      = true
LDFLAGS     = -G
OTHER_FLAGS = -Wall
ifeq ($(OS), SOLARIS_64)
DEFINES     = -DSOLARIS
CFLAGS     += -m64
LDFLAGS    += -m64
endif
endif

#
# End of platform section.
##########################


ifeq ($(BUILD), OPT)
OTHER_FLAGS += -O3
DEFINES     += -DNDEBUG
else
OTHER_FLAGS += -g3
DEFINES     += -DDEBUG
endif

#
# database set
#
ifeq ($(HAVE_HIREDIS), true)
DEFINES     += -DHAVE_HIREDIS
DIRS		 += db/redis
LDFLAGS		 += -lhiredis
endif

ifeq ($(HAVE_LIBPQ), true)
DEFINES     += -DHAVE_LIBPQ
DIRS		 += db/postgresql
LDFLAGS		 += -lpq
endif

##########################
# Other possible defines:
# To use poll(2) instead of select(2) for events checking:
# DEFINES += -DUSE_POLL
# You may prefer to use select for applications that have many threads
# using one file descriptor, and poll for applications that have many
# different file descriptors.  With USE_POLL poll() is called with at
# least one pollfd per I/O-blocked thread, so 1000 threads sharing one
# descriptor will poll 1000 identical pollfds and select would be more
# efficient.  But if the threads all use different descriptors poll()
# may be better depending on your operating system's implementation of
# poll and select.  Really, it's up to you.  Oh, and on some platforms
# poll() fails with more than a few dozen descriptors.
#
# Some platforms allow to define FD_SETSIZE (if select() is used), e.g.:
# DEFINES += -DFD_SETSIZE=4096
#
# To use malloc(3) instead of mmap(2) for stack allocation:
# DEFINES += -DMALLOC_STACK
#
# To provision more than the default 16 thread-specific-data keys
# (but not too many!):
# DEFINES += -DST_KEYS_MAX=<n>
#
# To start with more than the default 64 initial pollfd slots
# (but the table grows dynamically anyway):
# DEFINES += -DST_MIN_POLLFDS_SIZE=<n>
#
# Note that you can also add these defines by specifying them as
# make/gmake arguments (without editing this Makefile). For example:
#
# make EXTRA_CFLAGS=-DUSE_POLL <target>
#
# (replace make with gmake if needed).
#
# You can also modify the default selection of an alternative event
# notification mechanism. E.g., to enable kqueue(2) support (if it's not
# enabled by default):
#
# gmake EXTRA_CFLAGS=-DMD_HAVE_KQUEUE <target>
#
# or to disable default epoll(4) support:
#
# make EXTRA_CFLAGS=-UMD_HAVE_EPOLL <target>
#
##########################

INCS := -I"../include" -I"../deps/lua/src"
CFLAGS += $(DEFINES) $(OTHER_FLAGS) $(EXTRA_CFLAGS)
CFLAGS += $(INCS)

LIBS += -L"../deps/lua/src" -lpthread -lm -ldl -lcheck -llua
LDFLAGS += $(LIBS) -L$(TARGETDIR)

#DIRS = $(shell find . -maxdepth 3 -type d)
SRCS := $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))
#SRCS := $(filter-out $(EXCLUDE_FILES), $(SRCS))
SRCS := $(foreach file,$(SRCS),$(TARGETDIR)/$(file))
OBJS = $(patsubst %.c, %.o, $(SRCS))
OBJS += $(EXTRA_OBJS)

MAKEDIRS = $(TARGETDIR) $(foreach dir,$(DIRS),$(TARGETDIR)/$(dir))

none:
	@echo "Please do"
	@echo "   make PLATFORM"
	@echo "where PLATFORM is one of these:"
	@echo "   $(PLATS)"

all: $(MAKEDIRS) vine

$(MAKEDIRS):
	if [ ! -d $@ ]; then mkdir $@; fi

vine: $(OBJS)
	$(CC) -o $(TARGETDIR)/vine $(OBJS) $(LDFLAGS)

$(TARGETDIR)/st/md.o: st/md.S
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGETDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

echo_inner: 
	@echo ""
	@echo "These are the parameters currently set in src/Makefile to build vine:"
	@echo ""
	@echo "PLAT = $(PLAT)"
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "RM = $(RM)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "TARGETDIR = $(TARGETDIR)"
	@echo "OBJS = $(OBJS)"
	@echo ""

clean:
	rm -rf $(TARGETDIR)


#########################################################
# Target rules:

aix:
	$(MAKE) all OS="AIX" BUILD="OPT"

freebsd:
	$(MAKE) all OS="FREEBSD" BUILD="OPT"

linux:
	$(MAKE) all OS="LINUX" BUILD="OPT"

solaris:
	$(MAKE) all OS="SOLARIS_64" BUILD="OPT"

echo:
	$(MAKE) echo_inner OS=$(PLAT) BUILD="OPT"

#########################################################



cygwin-debug:
	$(MAKE) OS="CYGWIN" BUILD="DBG"
cygwin-optimized:
	$(MAKE) OS="CYGWIN" BUILD="OPT"

darwin-debug:
	$(MAKE) OS="DARWIN" BUILD="DBG"
darwin-optimized:
	$(MAKE) OS="DARWIN" BUILD="OPT"

hpux-debug:
	$(MAKE) OS="HPUX" BUILD="DBG"
hpux-optimized:
	$(MAKE) OS="HPUX" BUILD="OPT"
hpux-64-debug:
	$(MAKE) OS="HPUX_64" BUILD="DBG"
hpux-64-optimized:
	$(MAKE) OS="HPUX_64" BUILD="OPT"

irix-n32-debug:
	$(MAKE) OS="IRIX" BUILD="DBG"
irix-n32-optimized:
	$(MAKE) OS="IRIX" BUILD="OPT"
irix-64-debug:
	$(MAKE) OS="IRIX_64" BUILD="DBG"
irix-64-optimized:
	$(MAKE) OS="IRIX_64" BUILD="OPT"

# compatibility
linux-ia64-debug: linux-debug
linux-ia64-optimized: linux-optimized

netbsd-debug:
	$(MAKE) OS="NETBSD" BUILD="DBG"
netbsd-optimized:
	$(MAKE) OS="NETBSD" BUILD="OPT"

openbsd-debug:
	$(MAKE) OS="OPENBSD" BUILD="DBG"
openbsd-optimized:
	$(MAKE) OS="OPENBSD" BUILD="OPT"

osf1-debug:
	$(MAKE) OS="OSF1" BUILD="DBG"
osf1-optimized:
	$(MAKE) OS="OSF1" BUILD="OPT"

solaris-debug:
	$(MAKE) OS="SOLARIS" BUILD="DBG"
solaris-optimized:
	$(MAKE) OS="SOLARIS" BUILD="OPT"


##########################
