#include<semaphore.h>
#include"main.h"

/*Stucture of shared variables, opens by mmap()*/
typedef struct{
	/*Current visitor number, every visitor read value 
	 * into his local variable, after this do increment*/
	uint32_t* visitor_cn;

	/*How many visitors we expect during park open
	 *Cart */
	uint32_t* visitor_gl_cn;

	/*Current cart number*/
	uint32_t* cart_cn;

	/*Rather for case, when number of left visitors less then cart capacity*/
	uint32_t* cur_vis_on_board;

	/*Log/process ordinal number*/
	uint32_t* process_cn;

	/*After assembler experience this seems as a good idea
	 * Finish flag - we are about to finish, but not exatly now
	 * Cart sets flag, when it understands, that one going to be last ride*/
	uint32_t* finish_flag;

} shared_variables;

/*Sends signal from dispatcher to cart to start*/
sem_t* dis_next_cart; //sem(0)

/*Boarding station - critical section*/
sem_t* boarding_station; //sem(1)

/*Mutex for couting or printing*/
sem_t* mutex; //sem(1)

/*Queue of visitors*/
sem_t* queue; //sem(0)

/*Leaving station - critical section*/
sem_t* leaving_station; //sem(1)

/*Cart sends signal to visitor to start leaving*/
sem_t* visitor_leaving; //sem(0)

/*When i jut started, i was inspired by Little book of semaphore
 * And i wanted to implement turnstile for situation, 
 * When visitor boarding or leaving one by one,
 * And exatly when last has boarded or left - send a signal to continue
 * Unfortunatly, i don't realize how to implement*/
sem_t* boarding_turnstile; //sem(0)
sem_t* leaving_turnstile; //sem(0)

/*Signal from cart to dispatcher, that cart was boarded and can start*/
sem_t* boarding_signal; //sem(0)

/*Opened all semaphores*/
void all_semaphores_open(void);

/*Close all semaphores*/
void all_semaphores_close(void);

/*Implemented 3 types of processes*/
void dispatcher_semaphore(arguments *arg, shared_variables *sh_var);
void cart_semaphore(arguments *arg, shared_variables *sh_var);
void visitor_semaphore(arguments *arg, shared_variables *sh_var);

shared_variables* sh_var_ctor(void);

void sh_var_dtor(shared_variables *sh_var);

void process_branching(arguments *arg, shared_variables *sh_var);

int shared_variables_init(arguments *arg, shared_variables *sh_var);
