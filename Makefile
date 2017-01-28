CXX = g++
CXXFLAGS = -O2 -Wc++11-extensions

all: TriPeaksSolver FreeCellSolver PyramidSolver KlondikeSolver

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

fcs125: fcs125.cc common.o
	$(CXX) $(CXXFLAGS) -o $@ $< common.o

KlondikeSolver: KlondikeSolver.cc common.o
	$(CXX) $(CXXFLAGS) -o $@ $< common.o -DDEBUG

clean:
	rm -f *Solver *.o
