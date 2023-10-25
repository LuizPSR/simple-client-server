all: client server

client: src/client.c
	gcc -o bin/client src/client.c

server: src/server.c
	gcc -o bin/server src/server.c

clean:
	rm -f bin/client bin/server

.DEFAULT_GOAL := all
