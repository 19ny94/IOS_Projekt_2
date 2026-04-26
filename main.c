#include<stdio.h>
#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<semaphore.h>
#include<unistd.h>
#include"main.h"
#include"semaphores.h"

/*constructor of arguments structure*/
arguments* arg_strc_ctor(void){
	arguments *arg = malloc(sizeof(arguments));
	if(arg == NULL){return NULL;}
	return arg;
}

shared_variables* sh_var_ctor(void){
	
	shared_variables *sh_var = malloc(sizeof(shared_variables));
	if(sh_var == NULL){return NULL;}
	return sh_var;
}

void arg_strc_dtor(arguments *arg){
	free(arg);
}

void sh_var_dtor(shared_variables *sh_var){
	free(sh_var);
}

/* function reading and asserting arguments */
uint8_t arguments_reading(int argc, char **argv, arguments *arg){
	if(argc != 7){fprintf(stderr, "Invalid numbers of arguments\n"); return 1;}

	arg->card = atoi(argv[1]);
	if(arg->card >= 10 || arg->card <= 0) {goto error_state;}

	arg->visitor = atoi(argv[2]);
	if(arg->visitor >= 1000 || arg->visitor <= 0) {goto error_state;}

	arg->card_capacity = atoi(argv[3]);
	if(arg->card_capacity > 40 || arg->card_capacity < 4) {goto error_state;}

	arg->card_time = atoi(argv[4]);
	if(arg->card_time > 1000 || arg->card_time < 0) {goto error_state;}

	arg->visitor_time = atoi(argv[5]);
	if(arg->visitor_time > 1000 || arg->visitor_time < 0) {goto error_state;}

	arg->safe_interval = atoi(argv[6]);
	if(arg->safe_interval > 100 || arg->safe_interval <= 0) {goto error_state;}

	return 0;

error_state:
	fprintf(stderr, "Arguments parsing error\n");
	return 1;
}

void semaphores_implement(arguments *arg, shared_variables *sh_var){

	sh_var->visitor_num = 
	mmap(NULL, sizeof(uint16_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->visitor_ts_count = 
	mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->card_num = 
	mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	sh_var->visitor_cnt = 
	mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, arg->visitor);
}

int main(int argc, char **argv){

int8_t return_value = 0;

arguments *arg = arg_strc_ctor();
if(arg == NULL){return 1;}

shared_variables *sh_var = sh_var_ctor();
if(sh_var == NULL){return 1;}

if(arguments_reading(argc, argv, arg) != 0){return_value = 1;}

arg_strc_dtor(arg);
sh_var_dtor(sh_var);

return return_value;
}
