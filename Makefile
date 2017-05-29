# THIS IS AN OUTDATED MAKEFILE
# PLEASE INSTALL BUCK (https://buckbuild.com) AND BUILD //:calc

LLVM_CONFIG=/usr/local/opt/llvm/bin/llvm-config
CXX=clang++
CXXFLAGS=-std=c++11 `${LLVM_CONFIG} --cxxflags`
LINK.o=$(LINK.cc)
LDFLAGS=`${LLVM_CONFIG} --system-libs --ldflags --libs engine`

calc: calc.o parser.o

calc.o: calc.cpp
parser.o: parser.cpp

clean:
	rm calc *.o
