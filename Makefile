CXXFLAGS = -std=c++14 -Wall -Werror -O0 -g
CXX = g++

OBJECTS = RunTests.o Hashes.o BucketsTable.o QuotientFilter.o

default: run-tests

run-tests: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

RunTests.o: RunTests.cpp Timing.h Hashes.h Timer.h BucketsTable.h QuotientFilter.h

%.o: %.cpp %.h Hashes.h

clean:
	rm -f run-tests *.o *~
