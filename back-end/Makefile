CFLAGS=-std=gnu++11 -pedantic -Wall -ggdb3
PROGS=testServerRequest daemon 
OBJS=$(patsubst %,%.o,$(PROGS)) *.o
EXTRAFLAGS=-lpqxx -lpq -pthread
GPB = `pkg-config --cflags --libs protobuf`
PROTO_CC = ./protobuf/world_amazon.pb.cc
PROTO_H = ./protobuf/world_amazon.pb.h

all: $(PROGS)
daemon: daemon.cpp Server.cpp client.cpp handleWorld.cpp Database.cpp ServerRequest.cpp $(PROTO_H) WareHouse.hpp handleProto.hpp
	g++ -g $(CFLAGS) -o daemon daemon.cpp Server.cpp client.cpp handleWorld.cpp Database.cpp ServerRequest.cpp $(PROTO_CC) $(EXTRAFLAGS) $(GPB)
	
## DEBUG ServerRequest.cpp
testServerRequest: testServerRequest.cpp ServerRequest.cpp
	g++ -g $(CFLAGS) -o testServerRequest testServerRequest.cpp ServerRequest.cpp

.PHONY: clean
clean:
	rm -f *~ $(PROGS) $(OBJS)
