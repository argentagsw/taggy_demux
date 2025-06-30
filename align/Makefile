OBJDIR = obj
BINDIR = ../bin
SHARED_LIB_DIR=lib
SRCDIR = src
INCDIR = inc

#LIBS = -lm
LIBS = -lm -lpthread -lalign
CC = gcc
CPP = g++
BASICFLAGS = -Wall -Wextra -Wpedantic -Wshadow -fPIC
MOREFLAGS = -Wcast-align -Wcast-qual -Wsync-nand -Wtrampolines -Wshift-overflow=2 -Wodr \
-Wformat-signedness -Wsuggest-final-types
EXTRAFLAGS = -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict \
-Wnull-dereference -Wdouble-promotion
#DEFINES = -DASSERTIVE -DLOCALLOC -DPARALLEL -DDEBUG
DEFINES = -DASSERTIVE -DLOCALLOC -DPARALLEL
#DEFINES = -DLOCALLOC -DPARALLEL
CCFLAGS = $(DEFINES) -O3 -g $(BASICFLAGS) $(EXTRAFLAGS) $(MOREFLAGS) \
				 -Wno-unused-function -Wjump-misses-init 
CPPFLAGS = $(DEFINES) -O3 -g $(BASICFLAGS) $(EXTRAFLAGS) $(MOREFLAGS) \
				 -Wno-unused-function 

TARGET = $(BINDIR)/align

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(wildcard $(SRCDIR)/*.c)) \
$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.opp, $(wildcard $(SRCDIR)/*.cpp))

HEADERS = $(wildcard $(INCDIR)/*.h)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
		mkdir -p $(OBJDIR)
		$(CC) $(CCFLAGS) -c $< -o $@ -I $(INCDIR)

$(OBJDIR)/%.opp: $(SRCDIR)/%.cpp $(HEADERS)
		mkdir -p $(OBJDIR)
		$(CPP) $(CPPFLAGS) -c $< -o $@ -I $(INCDIR)

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
		mkdir -p $(BINDIR)
		$(CPP) $(OBJECTS) -Wall -L $(SHARED_LIB_DIR) $(LIBS) -o $@

clean:
		-rm -f $(OBJDIR)/*.o $(OBJDIR)/*.opp
		-rm -f $(TARGET)

