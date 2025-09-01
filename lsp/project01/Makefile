CC = gcc
CFLAGS = -Wall -Wextra -Werror -g
OBJECTS = main.o shell.o tree.o arrange.o file.o
TARGET = ssu_cleanup

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

main.o: main.c
	$(CC) $(CFLAGS) -c $^

shell.o: shell.c
	$(CC) $(CFLAGS) -c $^

tree.o: tree.c
	$(CC) $(CFLAGS) -c $^

arrange.o: arrange.c
	$(CC) $(CFLAGS) -c $^

file.o: file.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm $(OBJECTS) $(TARGET)