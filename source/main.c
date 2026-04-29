#include<stdio.h>
#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<semaphore.h>
#include<unistd.h>
#include<sys/wait.h> //read more later
#include"semaphores.h"

#define filename "proj2.out"

/*constructor of arguments' structure*/
arguments* arg_strc_ctor(void){
	arguments *arg = malloc(sizeof(arguments));
	if(arg == NULL){return NULL;}
	return arg;
}

/*constructor of shared variables' structure*/
shared_variables* sh_var_ctor(void){
	
	shared_variables *sh_var = malloc(sizeof(shared_variables));
	if(sh_var == NULL){return NULL;}
	return sh_var;
}

/*destructor of arguments' structure*/
void arg_strc_dtor(arguments *arg){
	free(arg);
}
/*destructor of shared_variables' structure*/
void sh_var_dtor(shared_variables *sh_var){
	free(sh_var);
}

/* function reading and asserting arguments */
uint8_t arguments_reading(int argc, char **argv, arguments *arg){
	if(argc != 7){fprintf(stderr, "Invalid numbers of arguments\n"); return 1;}

	arg->cart_gl_cn = atoi(argv[1]);
	if(arg->cart_gl_cn >= 10 || arg->cart_gl_cn <= 0) {goto error_state;}

	arg->visitor_gl_cn = atoi(argv[2]);
	if(arg->visitor_gl_cn >= 1000 || arg->visitor_gl_cn <= 0) {goto error_state;}

	arg->cart_capacity = atoi(argv[3]);
	if(arg->cart_capacity > 40 || arg->cart_capacity < 4) {goto error_state;}

	arg->cart_time = atoi(argv[4]);
	if(arg->cart_time > 1000 || arg->cart_time < 0) {goto error_state;}

	arg->visitor_time = atoi(argv[5]);
	if(arg->visitor_time > 1000 || arg->visitor_time < 0) {goto error_state;}

	arg->safe_interval = atoi(argv[6]);
	if(arg->safe_interval > 100 || arg->safe_interval <= 0) {goto error_state;}

	return 0;

error_state:
	fprintf(stderr, "Arguments parsing error\n");
	return 1;
}

/*implementation process branching with fork()*/
void process_branching(arguments *arg, shared_variables *sh_var){


	pid_t dis_pid = fork();
		if(dis_pid == 0){
			dispatcher_semaphore(arg, sh_var);
			exit(0);
		}

	/*Loop makes arg->cart process*/
	for(uint8_t c_pid = 0; c_pid < arg->cart_gl_cn; c_pid++){
		pid_t cart_pid = fork();
			if(cart_pid == 0){
				cart_semaphore(arg, sh_var);
				exit(0);
			}
	}

	/*Loop makes arg->visitor process*/
	for(uint16_t n_pid = 0; n_pid < arg->visitor_gl_cn; n_pid++){
		pid_t nasv_pid = fork();
			if(nasv_pid == 0){
				visitor_semaphore(arg, sh_var);
				exit(0);
			}
	}
	while(wait(NULL) > 0);

}

void shared_variables_init(arguments *arg, shared_variables *sh_var){

	sh_var->visitor_cn = 
	mmap(NULL, sizeof(uint16_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->visitor_ts_cn = 
	mmap(NULL, sizeof(uint16_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->visitor_ts_lv = 
	mmap(NULL, sizeof(uint16_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->visitor_gl_cn = 
	mmap(NULL, sizeof(uint16_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->cart_cn = 
	mmap(NULL, sizeof(uint16_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->cart_gl_cn = 
	mmap(NULL, sizeof(uint16_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->process_cn = 
	mmap(NULL, sizeof(uint16_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->finish_flag = 
	mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);
	
	sh_var->cur_vis_on_board = 
	mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	/*Init start values of variables*/
	*sh_var->visitor_gl_cn = arg->visitor_gl_cn;
	*sh_var->visitor_ts_cn = 0;
	*sh_var->visitor_ts_lv = 0;
	*sh_var->cart_gl_cn = arg->cart_gl_cn;
	*sh_var->finish_flag = 0;
	*sh_var->visitor_cn = 0;
	*sh_var->cart_cn = 0;
	*sh_var->process_cn = 0;
	arg->visitor_sleep = rand() % (arg->visitor_time + 1);
	arg->cart_sleep = (arg->cart_time/2) + rand() % (arg->cart_time - arg->cart_time/2 + 1);


}

int main(int argc, char **argv){

	/**/
	freopen(filename, "w", stdout);
	setbuf(stdout,NULL);

	/*Structs constructe*/
	arguments *arg = arg_strc_ctor();
	if(arg == NULL){return 1;}
	shared_variables *sh_var = sh_var_ctor();
	if(sh_var == NULL){return 1;}

	int8_t return_value = 0;

	if(arguments_reading(argc, argv, arg) != 0){return_value = 1;}

	shared_variables_init(arg, sh_var);

	all_semaphores_open();
	process_branching(arg, sh_var);
	all_semaphores_close();

	arg_strc_dtor(arg);
	sh_var_dtor(sh_var);

	if(return_value != 0){
		fprintf(stderr, "Neco se pokazilo :c \n");
	}

	return return_value;
}
