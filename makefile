all: server client
	
server: server.o util.o
	gcc server.o util.o -o server

client: client.o util.o
	gcc client.o util.o -o client

server.o: server.c util.o
	gcc -c server.c -o server.o

client.o: client.c util.o
	gcc -c client.c -o client.o

util.o: util.h util.c
	gcc -c util.c -o util.o
