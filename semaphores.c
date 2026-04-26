#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>
#include"semaphores.h"

void all_semaphores_open(void){
	/*sem_open(const char *name )*/
	dis_to_card_boarding =
	sem_open("/dis_to_card_boarding", O_CREAT ,0644, 1);
	
	card_to_vis_boarding =
	sem_open("/card_to_vis_boarding", O_CREAT ,0644, 0);

	card_boarding_wait =
	sem_open("/card_boarding_wait", O_CREAT ,0644, 1);
	
	turnstile_visitor_1 =
	sem_open("/turnstile_visitor_1", O_CREAT ,0644, 0);

	turnstile_visitor_2 =
	sem_open("/turnstile_visitor_2", O_CREAT ,0644, 0);
	
	mutex =
	sem_open("/mutex", O_CREAT ,0644, 1);

	card_to_vis_leaving =
	sem_open("/card_to_vis_leaving", O_CREAT ,0644, 0);
	
	queue =
	sem_open("/queue", O_CREAT ,0644, 0);

	cs_boarding_platform =
	sem_open("/cs_boarding_platform", O_CREAT ,0644, 1);

	cs_leaving_platform=
	sem_open("/cs_leaving_platform", O_CREAT ,0644, 1);

	dis_next_card =
	sem_open("/dis_next_card", O_CREAT ,0644, 1);

	sem_card_leaving = 
	sem_open("/sem_card_leaving", O_CREAT ,0644, 0);
}

void close_all_semaphores(void){
	sem_close(dis_to_card_boarding);
	sem_close(card_to_vis_boarding);
	sem_close(card_boarding_wait);
	sem_close(turnstile_visitor_1);
	sem_close(turnstile_visitor_2);
	sem_close(mutex);
	sem_close(card_to_vis_leaving);
	sem_close(queue);
	sem_close(cs_boarding_platform);
	sem_close(cs_leaving_platform);
	sem_close(dis_next_card);
}

void dispatcher_semaphore(arguments *arg){
	
	fprintf(stdout, "D: started");

	if(arg->visitor != 0){
	/*dispatcher send signal to card to start boarding*/
	sem_post(dis_to_card_boarding);

	sem_wait(dis_next_card);
	fprintf(stdout, "D: next card");
	usleep(arg->safe_interval);
	}

	else
	{
	fprintf(stdout, "D: closing");
	}
}

void visitor_semaphore(arguments *arg, shared_variables *sh_var){

	/*Shared variable for visitor_current_number*/
	sem_wait(mutex);
	(*sh_var->visitor_num)++;
	sem_post(mutex);

	/*Shared variable for turnstile visitors counter*/

	/*Visitor enters to the platform*/
	fprintf(stdout, "N %d: started\n", *sh_var->visitor_num);

	/*Visitors are just walking in the park,
	 * so it will take time, before they
	 * come to queue*/
	
	/*Time, which visitor will take to come to queue*/
	uint16_t visitor_wait = rand() % arg->visitor_time + 1;
	usleep(visitor_wait);

	/*Visitors start and go to queue*/
	fprintf(stdout, "N %d: queue\n", *sh_var->visitor_num);

	/*Wait signal form card to start boarding*/
	sem_wait(queue);

	/*Every visitor come to mutex for count*/
	sem_wait(mutex);
	
	/*Visitor inform, they he`s boarding*/
	fprintf(stdout, "N %d: boarding\n", *sh_var->visitor_num);

	/*Visitor's counter increment*/
	(*sh_var->visitor_ts_count)++;

	/*When all visitors have already boarded -> send a signal*/
	if(*sh_var->visitor_rest >= arg->card_capacity){
		if(*sh_var->visitor_ts_count == arg->card_capacity){
			sem_post(card_boarding_wait);
			sem_wait(turnstile_visitor_2);
			sem_post(turnstile_visitor_1);
		}
	}
	else{
		if(*sh_var->visitor_ts_count == *sh_var->visitor_rest){
			sem_post(card_boarding_wait);
			sem_wait(turnstile_visitor_2);
			sem_post(turnstile_visitor_1);
		}

	}
	sem_post(mutex);
	sem_wait(turnstile_visitor_1);
	sem_post(turnstile_visitor_1);

	/*Here first turntile ends. Visitors take a ride.
	 * Now they are waiting signal from card, that they 
	 * arrivied to the leaving platform*/
	sem_wait(card_to_vis_leaving);

	fprintf(stdout, "N %d: leaving\n", *sh_var->visitor_num);

	/*Second part of turnstile*/
	sem_wait(mutex);
	(*sh_var->visitor_ts_count)--;
	if(*sh_var->visitor_ts_count == 0){

		/*Visitors send signal, they all of them
		 * have already left*/
		sem_post(sem_card_leaving);

		sem_wait(turnstile_visitor_1);
		sem_post(turnstile_visitor_2);
	}
	sem_post(mutex);
	sem_wait(turnstile_visitor_2);
	sem_post(turnstile_visitor_2);

	(*sh_var->visitor_cnt)--;
}



void card_semaphore(arguments *arg, shared_variables *sh_var){

	sem_wait(mutex);
	(*sh_var->card_num)++;
	sem_post(mutex);

	/*Every card starts once, and it not depends on dispatcher signal*/
	fprintf(stdout, "V %d: started\n", *sh_var->card_num);
	
	/*Card wait signal from dispatcher to start boarding*/
	sem_wait(dis_to_card_boarding);

	/*While card is boarding, his capacity decrease*/
	fprintf(stdout, "V %d: boarding started\n", *sh_var->card_num);

	/*Send signal to visitors to start boarding*/
	for(uint16_t idx = 0; idx < arg->card_capacity; idx++){
	sem_post(queue);
	}

	/*Wait while all visitors are boarding*/
	sem_wait(card_boarding_wait);

	/*Card waits signal from visitors turnstile*/
	fprintf(stdout, "V %d: boarding complete\n", *sh_var->card_num);

	/*Ride wiil take random time in this range*/
	uint16_t card_wait= rand() % (arg->card_time - (arg->card_time/2) + 1);
	usleep(card_wait);

	/*Critical section of leaving platform*/
	sem_post(card_to_vis_leaving);
	sem_wait(cs_leaving_platform);

	/*Card inform, that it`s waiting for all visitors leave*/
	fprintf(stdout, "V %d: leaving started\n", *sh_var->card_num);
	
	/*Wait signal from visitors*/
	sem_wait(sem_card_leaving);

	/*When all visitors left -> card go to queue of cards*/
	fprintf(stdout, "V %d: leaving complete\n", *sh_var->card_num);
	sem_post(cs_leaving_platform);

	sem_wait(mutex);
	*sh_var->visitor_rest -= arg->card_capacity;
	sem_post(mutex);

	sem_post(dis_next_card);
}
