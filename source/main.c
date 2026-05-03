/* @author Oleksandr Andreichenko (xandreo00)*/

#include<stdio.h>
#include<stdint.h>
#include<time.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<semaphore.h>
#include<unistd.h>
#include<sys/wait.h>
#include"semaphores.h"

/*Filename according to project*/
#define filename "proj2.out"

/*I put main as a first function just for code readability*/
int main(int argc, char **argv){

	/*When i started, all semaphores' prints i was doing into stdout
	 * freopen redirects standard I/O*/
	freopen(filename, "w", stdout);

	/*C doesn't write logs in buffer, but instantly in file*/
	setbuf(stdout,NULL);

	/*Specified return value, main function return value after all controls*/
	int8_t return_value = 0;

	/*Structs construct*/
	arguments *arg = arg_strc_ctor();
	if(arg == NULL){return_value = 1;}

	if(arguments_reading(argc, argv, arg) != 0){return_value = 1;}

	shared_variables *sh_var = sh_var_ctor(arg);
	if(sh_var == NULL){return_value = 1;}

	/*Semaphores work*/
	if(return_value == 0){
		if(all_semaphores_open() != 0){return_value = 1;}
			process_branching(arg, sh_var);
		if(all_semaphores_close() != 0){return_value = 1;}
	}

	/*Structures destruct*/
	arg_strc_dtor(arg);
	sh_var_dtor(sh_var);

	if(return_value != 0){
		fprintf(stderr, "Neco se pokazilo :c \n");
	}

	return return_value;
}

/*constructor of arguments' structure*/
arguments* arg_strc_ctor(void){

	arguments *arg = malloc(sizeof(arguments));
	if(arg == NULL){return NULL;}
	return arg;
}

/*constructor of shared variables' structure*/
shared_variables* sh_var_ctor(arguments *arg){
	
	/*i used to do malloc for this structure, but i can do 
	 * whole structure as a shared resource
	 * Also I used to allocate every shared variable separately */

	/*First argument - starting mapping address, i don't care so NULL
	 *Second - size_t of memory allocation
	 *Third - protection, read & write access
	 *Fourth - flags, MAP_ANON(doesn't link to file), MAP_SHARED - shared memory part
	 *Fifth - descriptor, doesn't care if set MAP_ANON
	 * Sixth - offset, 0 because start from start*/
	shared_variables *sh_var =
	mmap(NULL, sizeof(shared_variables), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	if(sh_var == MAP_FAILED){return NULL;}

	/*Init values*/
	sh_var->visitor_gl_cn = arg->visitor_gl_cn;
	sh_var->finish_flag = 0;
	sh_var->visitor_cn = 0;
	sh_var->cart_cn = 0;
	sh_var->process_cn = 0;

	return sh_var;

}

/*destructor of arguments' structure*/
void arg_strc_dtor(arguments *arg){
	free(arg);
}
/*destructor of shared_variables' structure*/
void sh_var_dtor(shared_variables *sh_var){
	munmap(sh_var, sizeof(shared_variables));

}

/* function reading and asserting arguments */
uint8_t arguments_reading(int argc, char **argv, arguments *arg){

	if(argc != 7){fprintf(stderr, "Invalid number of arguments\n"); return 1;}

	arg->cart_gl_cn = atoi(argv[1]);
	if(arg->cart_gl_cn <= 0 || arg->cart_gl_cn >= 10) {goto error_state;}

	arg->visitor_gl_cn = atoi(argv[2]);
	if(arg->visitor_gl_cn <= 0 || arg->visitor_gl_cn >= 10000) {goto error_state;}

	arg->cart_capacity = atoi(argv[3]);
	if(arg->cart_capacity < 4 || arg->cart_capacity > 40) {goto error_state;}

	arg->cart_time = atoi(argv[4]);
	if(arg->cart_time > 1000) {goto error_state;}

	arg->visitor_time = atoi(argv[5]);
	if(arg->visitor_time > 1000) {goto error_state;}

	arg->safe_interval = atoi(argv[6]);
	if(arg->safe_interval <= 0 || arg->safe_interval > 100) {goto error_state;}

	return 0;

error_state:
	fprintf(stderr, "Arguments parsing error\n");
	return 1;
}

/*implementation process branching with fork()*/
void process_branching(arguments *arg, shared_variables *sh_var){

	/*Every process inside the loop generate its own random number
	 * I use time(NULL), which returns UNIX time counter(that's classic use)
	 * but processes are doing too fast, so time(NULL) doesn't generate
	 * random number as well. So, as an idea, that number of processes
	 * must influence on its random number*/

	/*While i was testing my program, i faced a problem, 
	 * that some system, as my MacOS or Merlin has
	 * process limit, and if i test my program with 
	 * large numbers - it's about to stop, cause 
	 * processes can't be made. So I check if 
	 * process fail and which process was last*/

	pid_t dis_pid = fork();
		if(dis_pid == 0){
			srand(time(NULL) + getpid());
			dispatcher_semaphore(arg, sh_var);
			exit(0);
		}
		else if(dis_pid < 0){fprintf(stderr ,"dispatcher finished\n"); exit(1);}

	/*Loop makes arg->cart process*/
	for(uint8_t c_pid = 0; c_pid < arg->cart_gl_cn; c_pid++){
		pid_t cart_pid = fork();
			if(cart_pid == 0){
				srand(time(NULL) + getpid());
				arg->cart_sleep = (arg->cart_time/2) + rand() % (arg->cart_time - arg->cart_time/2 + 1);
				cart_semaphore(arg, sh_var);
				exit(0);
			}
			else if(cart_pid < 0){fprintf(stderr, "%d-c_process finished\n", c_pid); exit(1);}
	}

	/*Loop makes arg->visitor process*/
	for(uint16_t n_pid = 0; n_pid < arg->visitor_gl_cn; n_pid++){
		pid_t nasv_pid = fork();
			if(nasv_pid == 0){
				srand(time(NULL) + getpid());
				arg->visitor_sleep = rand() % (arg->visitor_time + 1);
				visitor_semaphore(arg, sh_var);
				exit(0);
			}
			else if(nasv_pid < 0){fprintf(stderr, "%d-n_process finished\n", n_pid); exit(1);}
	}

	while(wait(NULL) > 0);

}
