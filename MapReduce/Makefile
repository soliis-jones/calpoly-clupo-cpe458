CC = mpicc
FLAGS = -Wall -Werror -g -lm 
TARGET = mapreduce

all: $(TARGET)

$(TARGET): mapreduce.c bin
	$(CC) $(FLAGS) -L. -lmrmpi mapreduce.c libmrmpi.so -o $(TARGET) 

clean:
	rm -rf $(TARGET) *.o

