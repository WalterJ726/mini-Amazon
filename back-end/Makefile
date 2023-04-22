CFLAGS=-std=gnu++11 -pedantic -Wall -ggdb3
PROGS=daemon
OBJS=$(patsubst %,%.o,$(PROGS)) *.o
EXTRAFLAGS=-lpqxx -lpq -pthread
GPB = `pkg-config --cflags --libs protobuf`
PROTO_CC = ./protobuf/world_amazon.pb.cc
PROTO_H = ./protobuf/world_amazon.pb.h

all: $(PROGS)
daemon: daemon.cpp Server.cpp client.cpp handleWorld.cpp $(PROTO_H) WareHouse.hpp handleProto.hpp
	g++ -g $(CFLAGS) -o daemon daemon.cpp Server.cpp client.cpp handleWorld.cpp $(PROTO_CC) $(EXTRAFLAGS) $(GPB)
	
.PHONY: clean
clean:
	rm -f *~ $(PROGS) $(OBJS)
