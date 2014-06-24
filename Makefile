CXX=clang++
CXXFLAGS=-std=c++11 `llvm-config --cxxflags`
LINK.o=$(LINK.cc)
LDFLAGS=`llvm-config --ldflags --libs jit interpreter nativecodegen`

calc: calc.o parser.o

calc.o: calc.cpp
parser.o: parser.cpp

clean:
	rm calc *.o