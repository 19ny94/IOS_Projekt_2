CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

all: proj2.out

proj2.out: main.o semaphores.o 
#proj2 is not purpose of project, program do print in to file, proj2 just binarny file
	$(CC) $(CFLAGS) main.o semaphores.o -o proj2
main.o: main.c 
	$(CC) $(CFLAGS) -c main.c
semaphores.o: semaphores.c
	$(CC) $(CFLAGS) -c semaphores.c 

test_case: 
	./proj2.out 1 1 4 1 1 1
fail_case: 
	./proj2.out 1 1 4 1 1 0
