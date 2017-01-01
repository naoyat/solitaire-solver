CXX = g++
CXXFLAGS = -O2

all: TriPeaksSolver FreeCellSolver PyramidSolver

md5.o: md5.cc md5.h
	$(CXX) $(CXXFLAGS) -c $<

common.o: common.cc common.h
	$(CXX) $(CXXFLAGS) -c $<

TriPeaksSolver: TriPeaksSolver.cc common.o
	$(CXX) $(CXXFLAGS) -o $@ $< common.o

PyramidSolver: PyramidSolver.cc common.o
	$(CXX) $(CXXFLAGS) -o $@ $< common.o

FreeCellSolver: FreeCellSolver.cc common.o
	$(CXX) $(CXXFLAGS) -o $@ $< common.o

clean:
	rm -f *Solver *.o
