CC = gcc
CFLAGS = -Wall -Wextra -g
OBJECTS = ext2.o help.o main.o print.o shell.o tree.o
TARGET = ssu_ext2

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

main.o: main.c
	$(CC) $(CFLAGS) -c $<

ext2.o: ext2.c ext2.h
	$(CC) $(CFLAGS) -c $<

help.o: help.c help.h
	$(CC) $(CFLAGS) -c $<

print.o: print.c print.h
	$(CC) $(CFLAGS) -c $<

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c $<

tree.o: tree.c tree.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(OBJECTS) $(TARGET)
	rm *.gch