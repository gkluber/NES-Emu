CFLAGS=-g -std=gnu99 -O2 -Werror -Wall
CPP_FLAGS = --std=c++17 -O3 -Wall -Werror -w
LINK_FLAGS = -lSDL2main -lSDL2 

CPP_FILES=$(wildcard *.cpp)
CPP_O_FILES=$(subst .cpp,.o,$(CPP_FILES))

CC_FILES=$(wildcard *.cc)
CC_O_FILES=$(subst .cc,.o,$(CC_FILES))

C_FILES=$(wildcard *.c)
C_O_FILES=$(subst .cc,.o,$(C_FILES))

O_FILES = $(sort $(CPP_O_FILES) $(CC_O_FILES) $(C_O_FILES))

all : nes

nes : Makefile $(O_FILES)
	c++ -o nes $(O_FILES) $(LINK_FLAGS)

$(CPP_O_FILES) : %.o : Makefile %.cpp
	g++ $(CPP_FLAGS) -MD -o $*.o -c $*.cpp

$(CC_O_FILES) : %.o : Makefile %.cc
	g++ $(CPP_FLAGS) -MD -o $*.o -c $*.cc

$(C_O_FILES) : %.o : Makefile %.c
	gcc $(C_FLAGS) -MD -o $*.o -c $*.c

$(OCFILES) : %.o : %.c Makefile
	gcc $(CFLAGS) -MD -c $*.c

clean :
	rm -f *.d *.o

-include *.d
