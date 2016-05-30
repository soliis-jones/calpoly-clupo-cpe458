CC = mpicc
FLAGS = -Wall -Werror -g -lm 
TARGET = mapreduce

all: $(TARGET)

$(TARGET): mapreduce.c
	$(CC) $(FLAGS) -L. -lmrmpi mapreduce.c libmrmpi.so -o $(TARGET) 

clean:
	rm -rf $(TARGET) cwordfreq *.o

