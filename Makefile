CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

all: proj2

proj2: source/main.o source/semaphores.o 
#proj2 is not purpose of project, program do print in to file, proj2 just binarny file
	$(CC) $(CFLAGS) source/main.o source/semaphores.o -lpthread -o proj2
main.o: main.c 
	$(CC) $(CFLAGS) -Isource -c source/main.c -o source/main.o
semaphores.o: semaphores.c
	$(CC) $(CFLAGS) -Isource -c source/semaphores.c -o source/semaphores.o

test: 
	./proj2 2 10 4 10 10 10
test_2: 
	./proj2 1 9999 4 10 10 10

#Delete binarys and objects files
rm:
	rm -rf source/semaphores.o
	rm -rf source/main.o
	rm -rf proj2
