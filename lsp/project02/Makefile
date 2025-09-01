CC = gcc
CFLAGS = -Wall -Wextra -g
OBJECTS = add.o arrange.o daemon.o daemonconfig.o daemonlist.o daemonlog.o file.o main.o modify.o remove.o shell.o show.o
TARGET = ssu_cleanupd

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

main.o: main.c debug.h
	$(CC) $(CFLAGS) -c $<

add.o: add.c add.h
	$(CC) $(CFLAGS) -c $<

arrange.o: arrange.c arrange.h
	$(CC) $(CFLAGS) -c $<

daemon.o: daemon.c daemon.h
	$(CC) $(CFLAGS) -c $<

daemonconfig.o: daemonconfig.c daemonconfig.h
	$(CC) $(CFLAGS) -c $<

daemonlist.o: daemonlist.c daemonlist.h
	$(CC) $(CFLAGS) -c $<

daemonlog.o: daemonlog.c daemonlog.h
	$(CC) $(CFLAGS) -c $<

file.o: file.c file.h
	$(CC) $(CFLAGS) -c $<

modify.o: modify.c modify.h
	$(CC) $(CFLAGS) -c $<

remove.o: remove.c remove.h
	$(CC) $(CFLAGS) -c $<
	
shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c $<

show.o: show.c show.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(OBJECTS) $(TARGET)
	rm *.gch