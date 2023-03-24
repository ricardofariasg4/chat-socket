CFLAGS = -Wall -std=c99
EXECUTAVEL = client server

all: $(EXECUTAVEL)

# regras de ligação
client: client.o rickwebtools.o
	gcc client.o rickwebtools.o -o client

server: server.o rickwebtools.o
	gcc server.o rickwebtools.o -o server

# regras de compilação
client.o: client.c rickwebtools.h
	gcc -c client.c $(CFLAGS)

server.o: server.c rickwebtools.h
	gcc -c server.c $(CFLAGS)

rickwebtools.o: rickwebtools.c rickwebtools.h
	gcc -c rickwebtools.c $(CFLAGS)

# remoção de arquivos temporários
debug:
	tar -cvf debug.tar *.c *.h Makefile
	scp debug.tar ricardofariasg4@34.171.10.1:~
	scp debug.tar ricardofariasg4@104.197.191.227:~

clean:
	rm -f *~ *.o

purge: clean
	rm -f $(EXECUTAVEL)

