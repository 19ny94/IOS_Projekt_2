#include<stdio.h>
#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>
#include"main.h"

/*constructor of arguments structure*/
arguments* arg_strc_ctor(void){
	arguments *arg = malloc(sizeof(arguments));
	if(arg == NULL){return NULL;}
	return arg;
}

void arg_strc_dtor(arguments *arg){
	free(arg);
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

int main(int argc, char **argv){
int8_t return_value = 0;

arguments *arg = arg_strc_ctor();

if(arguments_reading(argc, argv, arg) != 0){return_value = 1;}
arg_strc_dtor(arg);

return return_value;
}
