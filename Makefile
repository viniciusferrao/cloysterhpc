#
# Compiler flags
#
CC		= clang
CFLAGS		= -Wall -Wextra #-Werror
CXX		= clang++ --std=c++2a
CXXFLAGS	= $(CFLAGS)
LD		= clang++
LFLAGS		= -Wall

#
# Project files
#
SRCS = main.cpp functions.cpp headnode.cpp cluster.cpp #tui.cpp #install.cpp #file2.c file3.c file4.c
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
DUMMYCFLAGS = -D_DUMMY_
DUMMYCXXFLAGS = $(DUMMYCFLAGS)

#
# Libraries needed during dynamic linking
#
NEWT = -lnewt
BOOST = -lboost_system
PTHREAD = -lpthread
DYNLIBS = $(NEWT) $(BOOST) $(LPTHREAD) 

.PHONY: all clean debug prep release remake

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

$(DUMMYEXE): $(RELOBJS)
	$(LD) $(LFLAGS) $(RELLFLAGS) $(DYNLIBS) -o $(RELEXE) $^
	rm -f $(EXE)
	ln -s $(RELEXE)

$(DUMMYDIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(RELCFLAGS) $(DUMMYCXXFLAGS) -o $@ $<

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

clean:
	rm -f $(RELEXE) $(RELOBJS) $(DBGEXE) $(DBGOBJS) $(EXE)
	rm -rf $(RELDIR) $(DBGDIR)
	rm -rf *.dSYM
