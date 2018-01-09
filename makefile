CC = gcc
CFLAGS = -lprocps

default: a.out

a.out :	shell.o mypwd.o mycd.o myecho.o
	$(CC) -o a.out shell.o mypwd.o mycd.o myecho.o $(CFLAGS)

shell.o : shell.c mypwd.h mycd.h myecho.h
	$(CC) $(CFLAGS) -c shell.c

mypwd.o : mypwd.c mypwd.h
	$(CC) $(CFLAGS) -c mypwd.c

myecho.o : myecho.c myecho.h
	$(CC) $(CFLAGS) -c myecho.c

mycd.o : mycd.c mycd.h
	$(CC) $(CFLAGS) -c mycd.c

clean:
	rm a.out *.o  *~
