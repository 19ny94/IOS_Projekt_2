#include<stdio.h>
#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
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

	arg->vozik = atoi(argv[1]);
	if(arg->vozik >= 10 || arg->vozik <= 0) {goto chybovy_stav;}

	arg->navstevnik = atoi(argv[2]);
	if(arg->navstevnik >= 1000 || arg->navstevnik <= 0) {goto chybovy_stav;}

	arg->kapacita = atoi(argv[3]);
	if(arg->kapacita > 40 || arg->kapacita < 4) {goto chybovy_stav;}

	arg->vozik_time = atoi(argv[4]);
	if(arg->vozik_time > 1000 || arg->vozik_time < 0) {goto chybovy_stav;}

	arg->navstevnik_time = atoi(argv[5]);
	if(arg->navstevnik_time > 1000 || arg->navstevnik_time < 0) {goto chybovy_stav;}

	arg->odstup = atoi(argv[6]);
	if(arg->odstup > 100 || arg->odstup <= 0) {goto chybovy_stav;}

	return 0;

chybovy_stav:
	fprintf(stderr, "Arguments parsing error\n");
	return 1;
}

int main(int argc, char **argv){
int8_t return_value = 0;

arguments *arg = arg_strc_ctor();

if(arguments_reading(argc, argv, arg) != 0){return_value = 1;}
else{printf("Uspech\n");}

arg_strc_dtor(arg);

return return_value;
}
