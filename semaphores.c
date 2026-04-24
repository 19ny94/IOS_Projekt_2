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

/* TODO: Card semaphore
 *	sem_card(1)
 * 	Vozik vyjizdi do nastupiste 
 * 	sem_card_wait()
 * 	Dela deckerement
 * 	Prijizdi do nastupiste, navstevnici vchazeji
 * 	if(card_capacity == 0 || customer_counter == 0): GO!
 * 	sem_card_post()
 * */

void card_semaphore(arguments *arg){

	/*There are two semaphores for boarding and leaving
	 * Cause, only one card can currently be on platform*/
	sem_open(sem_card_boarding, 1);
	sem_open(sem_card_leaving, 1);
	
	/*Card starts to board*/
	sem_wait(&sem_card_boarding);
	
	fprintf(stdout, "V %d: started", arg->card_capacity);
	arg->card--;

	/*All cards gets their max. capacity as a local variable*/
	uint8_t current_card_capacity = arg->card_capacity;

	/*While card is boarding, his capacity decrease*/
	fprintf(stdout, "V %d: boarding started", arg->card_capacity);

	/*When card is full, all last visitor for today was boarded - start*/
	while(arg->card_capacity == 0 || arg->visitor == 0){
	current_card_capacity--;
	fprintf(stdout, "V %d: boarding", arg->card_capacity);
	}
	fprintf(stdout, "V %d: boarding complete", arg->card_capacity);

	/*Do signal for next one*/
	sem_post(&sem_card_boarding);
	
	/*Ride wiil take random time in this range*/
	uint16_t card_wait= rand() % (arg->card_time - (arg->card_time/2) + 1);
	usleep(card_wait);

	/*After this time card has already arrivied to the leaving platform
	 *I do semophore here, because it needs to wait for platforme release*/
	sem_wait(&sem_card_leaving);
	
	fprintf(stdout, "V %d: leaving started", arg->card_capacity);
	
	/*While card is not empty - wait*/
	while(current_card_capacity != arg->card_capacity ){
	fprintf(stdout, "V %d: leaving", arg->card_capacity);
	}

	fprintf(stdout, "V %d: leaving complete", arg->card_capacity);
	sem_post(&sem_card_leaving);

	arg->card++;
}


