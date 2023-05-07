CC=cc
RPC_SYSTEM=rpc.o

.PHONY: all format

all: rpc-client rpc-server

rpc-client: client.o $(RPC_SYSTEM)
	$(CC) -o $@ $^

rpc-server: server.o $(RPC_SYSTEM)
	$(CC) -o $@ $^

client.o: client.c rpc.h
	$(CC) -Wall  -c -o $@ $<

server.o: server.c rpc.h
	$(CC) -Wall  -c -o $@ $<

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) -Wall -c -o $@ $<

format:
	clang-format -style=file -i *.c *.h

clean:
	rm -f rpc-client rpc-server client.o server.o $(RPC_SYSTEM)