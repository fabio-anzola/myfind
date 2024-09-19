COMP = gcc
COMPFLAGS = -Wall -Werror

all: myfind

myfind: myfind.o
	$(COMP) -o myfind myfind.o

myfind.o: myfind.c
	$(COMP) $(COMPFLAGS) -c myfind.c

clean:
	rm -f myfind myfind.o
