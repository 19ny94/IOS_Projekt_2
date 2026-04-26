#include<semaphore.h>
#include"main.h"

typedef struct{
	uint16_t* visitor_num;
	uint16_t* visitor_rest;
	uint16_t* visitor_cnt;
	uint8_t* visitor_ts_count;
	uint8_t* card_num;
} shared_variables;

/*--- Semaphores ---*/

/*Dispatcher signal to cart to start boarding*/
sem_t* dis_to_card_boarding; //sem(1)

/*Card sem to visitors: visitors can board to card on the platform*/
sem_t* card_to_vis_boarding; //sem(0)

/*Card wait until all visitors boarding on card*/
sem_t* card_boarding_wait; //sem(0)

/*Turnstile to visitors, when all visitors boarded -> send signal to card to start ride
 * First turnstile for boarding*/
sem_t* turnstile_visitor_1; //sem(0)
/*Second turnstile for leaving*/
sem_t* turnstile_visitor_2; //sem(0)

/*Mutex for counting*/
sem_t* mutex; //sem(1)

/*Card wait while visitors leaving*/
sem_t* card_to_vis_leaving; //sem(0)

/*Queue of visitors*/
sem_t* queue; //sem(0)

/*There are two critical section: 
 * boarding platform and leaving platform.*/
sem_t* cs_boarding_platform; //sem(1)
sem_t* cs_leaving_platform; //sem(1)

/*Cart signal to dispatcher, that current cart finished*/
sem_t* dis_next_card; //sem(1)

sem_t* sem_card_leaving; //sem(0)

/*Opens all semaphores(cause sem_init doesn't work on Mac ¯\_(ツ)_/¯)*/
void all_semaphores_open(void);

/*close all semaphores*/
void close_all_semaphores(void);

/*Implemented semaphore logic for dispatcher*/
void dispatcher_semaphore(arguments *arg);

/*Implemented semaphore logic for visitors*/
void visitor_semaphore(arguments *arg, shared_variables *sh_var);

/*Implemented semaphore logic for card*/
void card_semaphore(arguments *arg, shared_variables *sh_var);

/*constructor of shared variables*/
shared_variables *sh_var_ctor(void);

/*destructor of shared variables' structure*/
void sh_var_dtor(shared_variables *sh_var);
