OBJS = selectROI
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
INCLUDE = include
CVINCLUDE = /usr/local/include/opencv2/
LOCALLIB = /usr/local/lib
LOCALINCLUDE = /usr/local/include
LIBS = -lm $(shell pkg-config --libs opencv) 
SRCFILE = selectROI.cpp
TARGET =  selectROI


$(TARGET):	$(SRCFILE)
	$(CXX) $(LFLAGS) $(SRCFILE) -I$(INCLUDE) -I$(CVINCLUDE) -L$(LOCALLIB) -I$(LOCALINCLUDE) $(LIBS) -o $(OBJS)

all: $(TARGET)

clean:
	rm -f $(OBJS)
