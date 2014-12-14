CC = g++
CFLAGS 	= -c -O3 -Wall -fPIC -I ./include/ 

#include OpenCV
CFLAGS 		+= $(shell pkg-config opencv --cflags) 
LDFLAGS 	+= $(shell pkg-config opencv --libs)

LDFLAGS += -lX11 -lXtst -lXext

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

















