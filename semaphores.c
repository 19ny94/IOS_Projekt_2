/*Important detail: NEVER work with semaphore_current value
 * use that only as debug tool. In practice semaphores
 * change their value too quick and accidiently(for developer)
 * so, if implementation needs, just work with signals and waits functions*/

/*expected 5-8 semaphores*/

/*semophore empty - no people in queue*/
/*semophore full - queue is full*/
#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include"semaphore.h"
#include<unistd.h>
#include<stdint.h>
#include"main.h" /*Just for test*/


/* NOTE:  One of semaphores gotta be exactly turnstile
	  Little book of semaphores(page 29) */


sem_t sem_dispatcher;
sem_t sem_visitor;
sem_t sem_card_boarding;
sem_t sem_card_leaving;


/*void dispatcher_semaphore(int *id, arguments *arg){
	print this in any cases
	fprintf(stdin, "%i: D: started", id);

		
	usleep(arg->odstup);
	fprintf(stdin, "%d: D: next card", &id);

	if(vozik == 0){
	fprintf(stdin, "%i: D: closing", id);
	}

}

TODO: Dispetcher semaphore
 	printf("Start") -- print this in all cases, even if counter_of_customers = 0, in this case, just immidiatle close
 	??? for(boarding_complete && vozik != 0){printf("next cart"); vozik_sem_signal()} ??? 
	if(counter_of_customers == 0): printf("closing")
*/

/* TODO: Visitor semaphore
 * 	There is must be impelement queue
 * 	Reference is Little book of semaphores pg. 45
 *	In any cases, when any visitor starts print "started"
 *	In queue for any visitors works n-cards
 *	So i wanna implement this as a leader and follower
 *
 * 	For card_capacity visitor we have one card
 * 	So that like pair in relation, when 
 * 	set of visitors and corresponding card
 *
 * 	GENIUS IDEA: When card starts, it sends signal
 * 	to queue of visitor, that they can start.
 * 	Now visitor can go to the turnstile,
 * 	and when they all sit in card ->
 * 	send signal to card, that it can start
 *
 * 	Queue implementation: 
 * 	Card: 
 *	
 *
 * 	*/

/*TODO: finish
void visitor_semaphore(arguments *arg){

	uint16_t visitor_number = 0;
	visitor_number++;
	fprintf(stdout, "N %d: started\n", visitor_number);

	uint16_t visitor_wait= rand() % (TODO);
	usleep(visitor_wait);

}
*/


void card_semaphore(arguments *arg){

	/*There are two semaphores for boarding and leaving
	 * Cause, only one card can currently be on platform*/

	/*TODO: Semaphores must be opened by another way and in another place*/
//	sem_open(sem_card_boarding, 1);
//	sem_open(sem_card_leaving, 1);
	
	/*Card starts to board*/
	sem_wait(&sem_card_boarding);
	
	fprintf(stdout, "V %d: started\n", arg->card_capacity);
	arg->card--;

	/*All cards gets their max. capacity as a local variable*/
	uint8_t current_card_capacity = arg->card_capacity;

	/*While card is boarding, his capacity decrease*/
	fprintf(stdout, "V %d: boarding started\n", arg->card_capacity);

	/*When card is full, all last visitor for today was boarded - start*/
	while(arg->card_capacity == 0 || arg->visitor == 0){
	current_card_capacity--;
	fprintf(stdout, "V %d: boarding\n", arg->card_capacity);
	}
	fprintf(stdout, "V %d: boarding complete\n", arg->card_capacity);

	/*Do signal for next one*/
	sem_post(&sem_card_boarding);
	
	/*Ride wiil take random time in this range*/
	uint16_t card_wait= rand() % (arg->card_time - (arg->card_time/2) + 1);
	usleep(card_wait);

	/*After this time card has already arrivied to the leaving platform
	 *I do semophore here, because it needs to wait for platforme release*/
	sem_wait(&sem_card_leaving);
	
	fprintf(stdout, "V %d: leaving started\n", arg->card_capacity);
	
	/*While card is not empty - wait*/
	while(current_card_capacity != arg->card_capacity ){
	fprintf(stdout, "V %d: leaving\n", arg->card_capacity);
	}

	fprintf(stdout, "V %d: leaving complete\n", arg->card_capacity);
	sem_post(&sem_card_leaving);

	arg->card++;
}


