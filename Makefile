#
# OS detection to select which C++ version should we use
# Enterprise Linux 8 does not support C++2a by default, we select C++17 instead,
# for everything else we default to C++2a
#
OS := $(shell uname -s)
OS_RELEASE := $(shell uname -r)
ifeq ($(OS),Linux)
	ifeq ($(filter el8%,$(OS_RELEASE)),)
		STDCXX = c++17
	else
		STDCXX = c++2a
	endif
else
	STDCXX = c++2a
endif

#
# Compiler flags
#
CC = clang
CFLAGS = -Wall -Wextra -Wno-unused-private-field -Wno-unused-parameter -Wno-unused-variable #-Werror
CXX = clang++ --std=$(STDCXX)
CXXFLAGS = $(CFLAGS)
LD = clang++
LFLAGS = -Wall

#
# Project files
#
SRCS = main.cpp functions.cpp headnode.cpp cluster.cpp xcat.cpp terminalui.cpp network.cpp connection.cpp os.cpp repos.cpp
OBJS = $(SRCS:.cpp=.o)
EXE  = main

#
# Debug build settings
#
DBGDIR = debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS = -fsanitize=address -g -O0 -D_DEBUG_
DBGCXXFLAGS = $(DBGCFLAGS)
DBGLFLAGS = -fsanitize=address -g

#
# Release build settings
#
RELDIR = release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS = -O2
RELCXXFLAGS = $(RELCFLAGS)
RELLFLAGS =

#
# Dummy flags
#
DUMMYDIR = dummy
DUMMYEXE = $(DUMMYDIR)/$(EXE)
DUMMYOBJS = $(addprefix $(DUMMYDIR)/, $(OBJS))
DUMMYCFLAGS = -D_DUMMY_
DUMMYCXXFLAGS = $(DUMMYCFLAGS)
DUMMYLFLAGS =

#
# Libraries needed during dynamic linking
#
NEWT = -lnewt
PTHREAD = -lpthread

# Boost
BOOST = -lboost_system
ifeq ($(OS),Linux)
	BOOSTTHREAD = -lboost_thread
	BOOSTLOG = -DBOOST_LOG_DYN_LINK -lboost_log -lboost_log_setup
else
	BOOSTTHREAD = -lboost_thread-mt
	BOOSTLOG = -DBOOST_LOG_DYN_LINK -lboost_log-mt -lboost_log_setup-mt
endif

DYNLIBS = $(NEWT) $(LPTHREAD) $(BOOST) $(BOOSTTHREAD) $(BOOSTLOG)

.PHONY: all osdetect clean debug dummy prep release remake

# Default build
all: release

#
# Debug rules
#
debug: prep $(DBGEXE)

$(DBGEXE): $(DBGOBJS)
	$(LD) $(LFLAGS) $(DBGLFLAGS) $(DYNLIBS) -o $(DBGEXE) $^
	rm -f $(EXE)
	ln -s $(DBGEXE)

$(DBGDIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(DBGCXXFLAGS) -o $@ $<

#
# Dummy rules
#
dummy: prep $(DUMMYEXE)

$(DUMMYEXE): $(DUMMYOBJS)
	$(LD) $(LFLAGS) $(DBGLFLAGS) $(DUMMYLFLAGS) $(DYNLIBS) -o $(DUMMYEXE) $^
	rm -f $(EXE)
	ln -s $(DUMMYEXE)

$(DUMMYDIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(DBGCXXFLAGS) $(DUMMYCXXFLAGS) -o $@ $<

#
# Release rules
#
release: prep $(RELEXE)

$(RELEXE): $(RELOBJS)
	$(LD) $(LFLAGS) $(RELLFLAGS) $(DYNLIBS) -o $(RELEXE) $^
	rm -f $(EXE)
	ln -s $(RELEXE)

$(RELDIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(RELCFLAGS) -o $@ $<

#
# Other rules
#
prep:
	@mkdir -p $(DBGDIR) $(RELDIR) $(DUMMYDIR)

remake: clean all

osdetect:
	@echo $(OS)
	@echo $(OS_RELEASE)
	@echo $(STDCXX)

clean:
	rm -f $(RELEXE) $(RELOBJS) $(DBGEXE) $(DBGOBJS) $(DUMMYEXE) $(DUMMYOBJS) $(EXE)
	rm -rf $(RELDIR) $(DBGDIR)
	rm -rf *.dSYM
