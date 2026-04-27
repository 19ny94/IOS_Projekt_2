CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

all: proj2

proj2: source/main.o source/semaphores.o 
#proj2 is not purpose of project, program do print in to file, proj2 just binarny file
	$(CC) $(CFLAGS) source/main.o source/semaphores.o -o proj2
main.o: main.c 
	$(CC) $(CFLAGS) -Isource -c source/main.c -o source/main.o
semaphores.o: semaphores.c
	$(CC) $(CFLAGS) -Isource -c source/semaphores.c -o source/semaphores.o

test_case: 
	./proj2.out 1 1 4 1 1 1
fail_case: 
	./proj2.out 1 1 4 1 1 0
