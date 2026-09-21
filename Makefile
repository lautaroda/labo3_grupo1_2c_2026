CC = gcc
CFLAGS = -std=c89 -pedantic -Wall -Werror
LIBS = -lpthread -lrt

all: entrada monitor

entrada: entrada.o estacionamiento.o
	$(CC) $(CFLAGS) entrada.o estacionamiento.o -o entrada $(LIBS)

monitor: monitor.o estacionamiento.o
	$(CC) $(CFLAGS) monitor.o estacionamiento.o -o monitor $(LIBS)

entrada.o: entrada.c estacionamiento.h
	$(CC) $(CFLAGS) -c entrada.c

monitor.o: monitor.c estacionamiento.h
	$(CC) $(CFLAGS) -c monitor.c

estacionamiento.o: estacionamiento.c estacionamiento.h
	$(CC) $(CFLAGS) -c estacionamiento.c

clean:
	rm -f *.o entrada monitor
