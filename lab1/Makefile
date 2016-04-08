CC = mpicc
FLAGS = -Wall -Werror -g
TARGET = ./bin/TSP

all: TSP

TSP: TSP.o bin
	$(CC) -g -o $(TARGET) TSP.o

TSP.o: TSP.c
	$(CC) $(FLAGS) -o TSP.o -c TSP.c

bin: 
	mkdir bin

run: 
	mpirun -n $(num) ./bin/TSP

clean:
	rm -f TSP.o ./bin/TSP
