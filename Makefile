CC = g++
CFLAGS 	= -c -pg  -Wall -I ./include/ 
#-O3 -ffast-math

#include OpenCV
CFLAGS 		+= $(shell pkg-config opencv --cflags) 
LDFLAGS 	+= $(shell pkg-config opencv --libs)

LDFLAGS += -lX11 -lXtst

SOURCES		= $(wildcard ./src/*.cpp)

OBJECTS		= $(SOURCES:.cpp=.o)

EXECUTABLE 	= gesture

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $(CURDIR)/$< -o $@

clean: 
	rm -rf ./src/*.o $(EXECUTABLE)

















