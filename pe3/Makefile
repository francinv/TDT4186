#Make file to compile and run the program

CC=gcc
TARGET=flush

all: #target main
	$(CC) src/core/flush.c src/core/linkedlist.c -o $(TARGET)
	./$(TARGET)

compile: 
	$(CC) src/core/flush.c src/core/linkedlist.c -o $(TARGET)

run:
	./$(TARGET)

clean:
	rm $(TARGET)