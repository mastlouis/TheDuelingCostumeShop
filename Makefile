#mastlouis #fxwhittall

CXX = gcc

CXXFLAGS = -g -Wall

TARGET = part1

all:	$(TARGET)

clean:
	rm part1

part1: part1.c
	$(CXX) $(CXXFLAGS) -pthread -o part1 part1.c -lm 
