# Makefile for Dun-geon

PROG = dun-geon
SRCDIR = src
OBJDIR = obj
DATADIR = data
CXX = g++
INCDIRS = $(shell find $(SRCDIR) -type d)
INCFLAGS = $(addprefix -I,$(INCDIRS))
LDFLAGS = -lncurses -lmenu
CXXFLAGS = -g -O2 -std=c++17 -Wall -pedantic $(INCFLAGS) $(DEPFLAGS)
DEPFLAGS = -MMD -MP
SRCS = $(shell find $(SRCDIR) -name *.cpp)
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

all: $(PROG)

$(PROG): $(OBJS) $(DATADIR)
	$(CXX) $(LDFLAGS) -o $(PROG) $(OBJS)

$(OBJDIR) $(DATADIR):
	@mkdir -p $@

%.o: %.cpp

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJDIR) $(PROG)

.PHONY: all clean

-include $(DEPS)