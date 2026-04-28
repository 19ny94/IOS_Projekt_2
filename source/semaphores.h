#include<semaphore.h>
#include"main.h"

typedef struct{
	uint16_t* visitor_cn;
	uint16_t* visitor_ts_cn;
	uint16_t* visitor_ts_lv;
	uint16_t* visitor_gl_cn;
	uint16_t* cart_cn;
	uint16_t* cart_gl_cn;
	uint16_t* cur_vis_on_board;
	uint16_t* process_cn;
	uint8_t* finish_flag;
} shared_variables;

sem_t* dis_next_card; //sem(0)
sem_t* boarding_station; //sem(1)
sem_t* mutex; //sem(1)
sem_t* visitor_boarding; //sem(0)
sem_t* leaving_station; //sem(1)
sem_t* visitor_leaving; //sem(0)
sem_t* boarding_turnstile; //sem(0)
sem_t* leaving_turnstile; //sem(0)
sem_t* boarding_signal; //sem(0)

void all_semaphores_open(void);
void close_all_semaphores(void);
void dispatcher_semaphore(arguments *arg, shared_variables *sh_var);
void cart_semaphore(arguments *arg, shared_variables *sh_var);
void visitor_semaphore(arguments *arg, shared_variables *sh_var);
