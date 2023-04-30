CFLAGS=-std=gnu++11 -pedantic -Wall -ggdb3
PROGS=testServerRequest daemon 
OBJS=$(patsubst %,%.o,$(PROGS)) *.o
EXTRAFLAGS=-lpqxx -lpq -pthread
PROTO_LIB_DIR = ./protobuf/lib
PROTO_HEADER_DIR = ./protobuf/include
GPB = `pkg-config --cflags --libs protobuf`
PROTO_CC = ./protobuf/world_amazon.pb.cc
PROTO_H = ./protobuf/world_amazon.pb.h
UPS_CC = ./protobuf/ups_amazon.pb.cc
UPS_H = ./protobuf/ups_amazon.pb.h

all: $(PROGS)
daemon: daemon.cpp Server.cpp client.cpp handleWorld.cpp handleUPS.cpp handleCustomer.cpp Database.cpp ServerRequest.cpp $(PROTO_H) $(UPS_H) WareHouse.hpp handleProto.hpp
	g++ -g $(CFLAGS) -o daemon daemon.cpp Server.cpp client.cpp handleWorld.cpp handleUPS.cpp handleCustomer.cpp Database.cpp ServerRequest.cpp $(PROTO_CC) $(UPS_CC) $(EXTRAFLAGS) $(GPB)
	
## DEBUG ServerRequest.cpp
testServerRequest: testServerRequest.cpp ServerRequest.cpp
	g++ -g $(CFLAGS) -o testServerRequest testServerRequest.cpp ServerRequest.cpp

.PHONY: clean
clean:
	rm -f *~ $(PROGS) $(OBJS)
