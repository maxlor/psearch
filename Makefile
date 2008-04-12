CXX=	g++
CXXFLAGS=	-O3
LDFLAGS=	-O3

.PHONY: clean run

psearch: psearch.o regexlist.o index.o
	${CXX} ${LDFLAGS} -o $@ $+

psearch.o: psearch.cpp index.h regexlist.h

regexlist.o: regexlist.cpp regexlist.h

index.o: index.cpp index.h


clean:
	rm -f psearch *.o

run: psearch
	./psearch abc
