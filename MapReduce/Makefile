CC = mpicc
FLAGS = -Wall -Werror -g -lm 
TARGET = mapreduce

all: $(TARGET) cwordfreq

$(TARGET): mapreduce.c bin
	$(CC) $(FLAGS) -L. -lmrmpi mapreduce.c libmrmpi.so -o $(TARGET) 

cwordfreq: cwordfreq.c
	$(CC) $(FLAGS) -L. -lmrmpi cwordfreq.c libmrmpi.so -o cwordfreq

clean:
	rm -rf $(TARGET) cwordfreq *.o

