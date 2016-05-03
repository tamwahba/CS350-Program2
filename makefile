CC = g++
CXXFLAGS = -std=c++11 -Wall -pedantic
OBJECTS = program2.o Block.o DataBlock.o IMap.o INode.o LFS.o Segment.o SummaryBlock.o

all:	program2

program2:	$(OBJECTS)
	$(CC) $(CXXFLAGS) $(OBJECTS) -o $@

%.o:	%.h	%.cpp
	$(CC) $(CXXFLAGS) -c $?

clean:
	rm -f *.o *.gch *.tar.gz program2

tar:	clean
	tar -zcvf program2.tar.gz *.cpp *.h README.md

