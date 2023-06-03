# Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
# hereby granted to students registered for University of Pennsylvania
# CIT 5950 for use solely during Spring Semester 2023 for purposes of
# the course.  No other use, copying, distribution, or modification
# is permitted without prior written consent. Copyrights for
# third-party components of this work must be honored.  Instructors
# interested in reusing these course materials should contact the
# author.

# define the commands we will use for compilation and library building
CC = gcc
CXX = g++

# define useful flags to cc/ld/etc.
CFLAGS += -g -Wall -Wpedantic -I. -I.. -std=c11 -O0
CXXFLAGS += -g -Wall -Wpedantic -I. -I.. -std=c++11 -O0
CPPUNITFLAGS = -L../gtest -lgtest

# define common dependencies
OBJS = LinkedList.o HashTable.o
HEADERS = LinkedList.h HashTable.h
TESTOBJS = test_linkedlist.o test_hashtable.o test_suite.o

# compile everything; this is the default rule that fires if a user
# just types "make" in the same directory as this Makefile
all: test_suite

test_suite: $(TESTOBJS) $(OBJS) 
	$(CXX) $(CFLAGS) -o test_suite $(TESTOBJS) \
	$(CPPUNITFLAGS) $(OBJS) -lpthread $(LDFLAGS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~ *.gcno *.gcda *.gcov test_suite
