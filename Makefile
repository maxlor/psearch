CXXFLAGS+=	-O3

all: psearch

psearch: psearch.o index.o
	${CXX} ${LDFLAGS} -o $@ $> $+

psearch.o: psearch.cpp index.h

index.o: index.cpp index.h

clean:
	rm -f psearch *.o
