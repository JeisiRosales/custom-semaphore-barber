CXX       := g++
CXXFLAGS  := -std=c++11 -Wall -Wextra -Iinclude -pthread
LDFLAGS   := -pthread

SRCDIR    := src
INCDIR    := include
BUILDDIR  := build
TARGET    := $(BUILDDIR)/barber_shop

SRCS      := $(wildcard $(SRCDIR)/*.cpp)
OBJS      := $(SRCS:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(BUILDDIR)/*.o $(TARGET)
