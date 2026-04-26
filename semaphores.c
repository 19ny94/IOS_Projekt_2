/*Important detail: NEVER work with semaphore_current value
 * use that only as debug tool. In practice semaphores
 * change their value too quick and accidiently(for developer)
 * so, if implementation needs, just work with signals and waits functions*/

/*expected 5-8 semaphores*/

/*semophore empty - no people in queue*/
/*semophore full - queue is full*/
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<semaphore.h>
#include"semaphore.h"
#include<unistd.h>
#include<stdint.h>
#include"main.h" /*Just for test*/


/* NOTE:  One of semaphores gotta be exactly turnstile
	  Little book of semaphores(page 29) */

/*Dispatcher sem to card, card go to boarding platform*/
sem_t dis_to_card_boarding; //sem(1)

/*Card sem to visitors: visitors can board to card on the platform*/
sem_t card_to_vis_boarding; //sem(0)

/*Card wait until all visitors boarding on card*/
sem_t card_boarding_wait; //sem(0)

/*Turnstile to visitors, when all visitors boarded -> send signal to card to start ride
 * First turnstile for boarding*/
sem_t turnstile_visitor_1; //sem(card-capacity)
/*Second turnstile for leaving*/
sem_t turnstile_visitor_2; //sem(card-capacity)

/*Mutex for counting*/
sem_t mutex; //sem(1)

/*Card wait while visitors leaving*/
sem_t card_to_vis_leaving; //sem(0)

/*Queue of visitors*/
sem_t queue; //sem(0)

/*There are two critical section: 
 * boarding platform and leaving platform.
 * Only one card at the one moment can 
 * boarding or leaving*/
sem_t cs_boarding_platform; //sem(1)
sem_t cs_leaving_platform; //sem(1)

sem_t dis_next_card; //sem(1)


void dispatcher_semaphore(arguments *arg){
	
	fprintf(stdout, "D: started");

	if(arg->visitors != 0){
	/*dispatcher send signal to card to start boarding*/
	sem_post(&dis_to_card_boarding);

	sem_wait(&dis_next_card);
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
	(sh_var->visitor_num)++;

	/*Shared variable for turnstile visitors counter*/
	uint8_t* visitor_ts_count = 
	mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, 
	MAP_ANON | MAP_SHARED, -1, 0);

	/*Visitor enters to the platform*/
	fprintf(stdout, "N %d: started\n", sh_var->visitor_num);

	/*Visitors are just walking in the park,
	 * so it will take time, before they
	 * come to queue*/
	
	/*Time, which visitor will take to come to queue*/
	uint16_t visitor_wait = rand() % arg->visitor_time + 1;
	usleep(visitor_wait);

	/*Visitors start and go to queue*/
	fprintf(stdout, "N %d: queue\n", sh_var->visitor_num);

	/*Wait signal form card to start boarding*/
	sem_wait(&queue);

	/*Every visitor come to mutex for count*/
	sem_wait(&mutex);
	
	/*Visitor inform, they he`s boarding*/
	fprintf(stdout, "N %d: boarding\n", sh_var->visitor_num);

	/*Visitor's counter increment*/
	(sh_var->visitor_ts_count)++;

	/*When all visitors have already boarded -> send a signal*/
	if(sh_var->visitor_ts_count == arg->card_capacity){
		sem_post(&card_boarding_wait);
		sem_wait(&turnstile_visitor_2);
		sem_post(&turnstile_visitor_1);
	}
	sem_post(&mutex);
	sem_wait(&turnstile_visitor_1);
	sem_post(&turnstile_visitor_1);

	/*Here first turntile ends. Visitors take a ride.
	 * Now they are waiting signal from card, that they 
	 * arrivied to the leaving platform*/
	sem_wait(&card_to_vis_leaving);

	fprintf(stdout, "N %d: leaving\n", sh_var->visitor_num);

	/*Second part of turnstile*/
	sem_wait(&mutex);
	(sh_var->visitors_ts_count)--;
	if(sh_var->visitors_ts_count == 0){

		/*Visitors send signal, they all of them
		 * have already left*/
		sem_post(&sem_card_leaving);

		sem_wait(&turnstile_visitor_1);
		sem_post(&turnstile_visitor_2);
	}
	sem_post(&mutex);
	sem_wait(&turnstile_visitor_2);
	sem_post(&turnstile_visitor_2);
}



void card_semaphore(arguments *arg, shared_variables *sh_var){

	(sh_var->card_num)++;

	/*Every card starts once, and it not depends on dispatcher signal*/
	fprintf(stdout, "V %d: started\n", sh_var->card_num);
	
	/*Card wait signal from dispatcher to start boarding*/
	sem_wait(&dis_to_cad_boarding);

	/*While card is boarding, his capacity decrease*/
	fprintf(stdout, "V %d: boarding started\n", sh_var->card_num);

	/*Send signal to visitors to start boarding*/
	sem_post(&queue);

	/*Wait while all visitors are boarding*/
	sem_wait(&card_boarding_wait);

	/*Card waits signal from visitors turnstile*/
	fprintf(stdout, "V %d: boarding complete\n", sh_var->card_num);

	/*Do signal for next one*/
	sem_post(&sem_card_boarding);
	
	/*Ride wiil take random time in this range*/
	uint16_t card_wait= rand() % (arg->card_time - (arg->card_time/2) + 1);
	usleep(card_wait);

	/*Critical section of leaving platform*/
	sem_post(&card_to_vis_leaving);
	sem_wait(&cs_leaving_platform);

	/*Card inform, that it`s waiting for all visitors leave*/
	fprintf(stdout, "V %d: leaving started\n", arg->card_capacity);
	
	/*Wait signal from visitors*/
	sem_wait(&sem_card_leaving);

	/*When all visitors left -> card go to queue of cards*/
	fprintf(stdout, "V %d: leaving complete\n", arg->card_capacity);
	sem_post(&cs_leaving_platform);

	sem_post(&dis_next_card);
}
