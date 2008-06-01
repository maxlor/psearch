CXXFLAGS+=	-O3

all: psearch

psearch: psearch.o regexlist.o index.o
	${CXX} ${LDFLAGS} -o $@ $>

psearch.o: psearch.cpp index.h regexlist.h

regexlist.o: regexlist.cpp regexlist.h

index.o: index.cpp index.h

clean:
	rm -f psearch *.o
