# Set compiler to use
CC=g++
CFLAGS=-g -I. -fpermissive
LDFLAGS=
OBJS=Main.o Matrix.o Tetris.o ttymodes.o #CTetris.o
DEBUG=0

all:: Main.exe

Main.exe: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS_TET) -lpthread -lncursesw

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean: 
	rm -f *.exe *.o *~ *.stackdump
