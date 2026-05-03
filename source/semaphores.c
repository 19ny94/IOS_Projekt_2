/* @author Oleksandr Andreichenko (xandreo00)*/

#include<stdio.h>
#include<stdarg.h>
#include<stdint.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include"semaphores.h"

#define SEMAPHORE_CN 9

/* --- Semaphores declaration --- */

/*Sends signal from dispatcher to cart to start*/
sem_t* dis_next_cart; //sem(0)

/*Boarding station - critical section*/
sem_t* boarding_station; //sem(1)

/*Mutex for counting or printing*/
sem_t* mutex; //sem(1)

/*Queue of visitors*/
sem_t* queue; //sem(0)

/*Leaving station - critical section*/
sem_t* leaving_station; //sem(1)

/*Cart sends signal to visitor to start leaving*/
sem_t* visitor_leaving; //sem(0)

/*When I just started, I was inspired by the Little Book of Semaphore
 * And I wanted to implement turnstile for this case, 
 * When visitor boarding or leaving one by one,
 * And exactly when last has boarded or left - send a signal to continue
 * Unfortunately, I don't realize how to implement*/
sem_t* boarding_turnstile; //sem(0)
sem_t* leaving_turnstile; //sem(0)

/*Signal from cart to dispatcher, that cart was boarded and can start*/
sem_t* boarding_signal; //sem(0)


	/*---Init semaphore structure ---
	 * Every semaphore in structure has its name,
	 * name in "/" and init value
	 * Easiest way to do it as a global variable*/
	semaphore_arr sem_ct_arr[] = {
	{&dis_next_cart, "/dis_next_cart", 0},
	{&boarding_station, "/boarding_station", 1},
	{&mutex, "/mutex", 1},
	{&queue, "/queue", 0},
	{&leaving_station, "/leaving_station", 1},
	{&boarding_turnstile ,"/boarding_turnstile", 0},
	{&visitor_leaving, "/visitor_leaving", 0},
	{&leaving_turnstile, "/leaving_turnstile", 0},
	{&boarding_signal, "/boarding_signal", 0}
	};

/*Open all semaphores*/
uint8_t all_semaphores_open(void){

	/*Just unlink all semaphores by his name
	 * and call sem_open(Bacause MacOS doesn't work with sem_init)*/
	for(uint8_t sem_idx = 0; sem_idx < SEMAPHORE_CN; sem_idx++){
		sem_unlink(sem_ct_arr[sem_idx].sem_name);
		*sem_ct_arr[sem_idx].sem_arr =
		/*first argument - name, exactly in "/", 
		 * second - O_CREAT - when this flag is specified, function creates semaphore, if it doesn't already exist
		 * third - 0644 - Unix flag for files, specified read & write access rights
		 * fourth - semaphore value*/
		sem_open(sem_ct_arr[sem_idx].sem_name, O_CREAT ,0644, sem_ct_arr[sem_idx].init_value);
		/*If fails for any reason - return value SEM_FAILED*/
		if(*sem_ct_arr[sem_idx].sem_arr == SEM_FAILED){
			return 1;
		}
	}
	return 0;
}

/*Function close and unlink all opened semaphores*/
uint8_t all_semaphores_close(void){

	for(uint8_t sem_idx = 0; sem_idx < SEMAPHORE_CN; sem_idx++){
		/*sem_close return -1 if it's unsuccess*/
		if(sem_close(*sem_ct_arr[sem_idx].sem_arr) != 0){return 1;}
	}
	return 0;
}

void print_act(sem_t* mutex, char *action_txt, uint32_t* variable){
	sem_wait(mutex);
	(*variable)++;
	fprintf(stdout, action_txt, *variable);
	sem_post(mutex);
}

void print_act_2v(sem_t* mutex, char *action_txt, uint32_t* variable, uint32_t* variable_2){
	sem_wait(mutex);
	(*variable)++;
	fprintf(stdout, action_txt, *variable, *variable_2);
	sem_post(mutex);
}

void dispatcher_semaphore(arguments *arg, shared_variables *sh_var){
	
	/*Dispatcher process notify its start*/
	print_act(mutex, "%d: D: started\n", &sh_var->process_cn);

	while(1){
		if(sh_var->visitor_gl_cn == 0){
		break;
		}
		/*Dispatcher send signal to next cart to start*/
		print_act(mutex, "%d: D: next cart\n", &sh_var->process_cn);
		sem_post(dis_next_cart);
		
		/*Dispatcher waits when cart leaves boarding station.
		 * After waiting for a safe time interval to send signal to the next cart*/
		sem_wait(boarding_signal);
		usleep(arg->safe_interval);
	
		/*Cart understands if it doesn't go full
		 * So in this case, dispatcher makes a signal, that
		 * if some cart is taking a ride now -
		 * They finish and close. For others
		 * cart, which is currently free,
		 * dispatcher just send signal*/
		if(sh_var->finish_flag != 0){
		break;
		}
	}
	
	print_act(mutex, "%d: D: closing\n", &sh_var->process_cn);
	for(uint8_t cart_to_lv = 0; cart_to_lv < arg->cart_gl_cn; cart_to_lv++){
	sem_post(dis_next_cart);
	}
}

void cart_semaphore(arguments *arg, shared_variables *sh_var){

	/*Assign number to current cart process*/
	sem_wait(mutex);
	(sh_var->cart_cn)++;
	uint32_t cur_cart_num = sh_var->cart_cn;
	sem_post(mutex);

	/*Cart process notify its' start*/
	print_act_2v(mutex, "%d: V %d: started\n", &sh_var->process_cn, &cur_cart_num);

	while(1){

	/*Cart waits for dispatcher signal to start boarding*/
	sem_wait(dis_next_cart);

	/* If there aren't any more visitors -> finish
	 * I did it here, because all semaphores, even if they
	 * just started by dispatcher to just close itself*/
	if(sh_var->visitor_gl_cn == 0){
	break;
	}

	/*As I got, boarding and leaving stations are critical sections in our case
	 * So, only one cart can currently be at either station*/
	sem_wait(boarding_station);
	
	print_act_2v(mutex, "%d: V %d: boarding started\n", &sh_var->process_cn, &cur_cart_num);

	uint32_t cur_vis_on_board = 0;

	/*Exactly when a certain number of visitors boarded, 
	 * this certain number we take away from visitor global counter
	 *
	 * So, for example, if the current cart takes the last visitors on a board,
	 * so it's gotta be the last cart for today
	 * Next one just finish, before it starts*/
	sem_wait(mutex);
	if(sh_var->visitor_gl_cn >= arg->cart_capacity){
		cur_vis_on_board = arg->cart_capacity;
	}
	else{
		cur_vis_on_board = sh_var->visitor_gl_cn;
		sh_var->finish_flag = 1;
	}
	sh_var->visitor_gl_cn -= cur_vis_on_board;
	sem_post(mutex);

	/*Cart sends signal to n-visitors, where n - the number of visitors, which can 
	 * now board*/
	for(uint8_t vis_sig = 0; vis_sig < cur_vis_on_board; vis_sig++){
	sem_post(queue);
	sem_wait(boarding_turnstile);
	}

	/*Cart free critical section and send signal to next one*/
	print_act_2v(mutex, "%d: V %d: boarding complete\n", &sh_var->process_cn, &cur_cart_num);

	/*Cart free station for next cart*/
	sem_post(boarding_station);

	/*Also send signal to dispatcher that the cart has already boarded*/
	sem_post(boarding_signal);

	/*We're leaving, fasten your seat belts*/
	usleep(arg->cart_sleep);

	/*Cart comes to leaving station*/
	sem_wait(leaving_station);
	
	print_act_2v(mutex, "%d: V %d: leaving started\n", &sh_var->process_cn, &cur_cart_num);

	/*Cart sends an instruction to visitors to leave*/
	for(uint8_t vis_sig = 0; vis_sig < cur_vis_on_board; vis_sig++){
		sem_post(visitor_leaving);
		sem_wait(leaving_turnstile);
	}
	print_act_2v(mutex, "%d: V %d: leaving complete\n", &sh_var->process_cn, &cur_cart_num);
	
	/*Cart free leaving station*/
	sem_post(leaving_station);
	}

	print_act_2v(mutex, "%d: V %d: closed\n", &sh_var->process_cn, &cur_cart_num);

}

void visitor_semaphore(arguments *arg, shared_variables *sh_var){

	/*Assign number to current visitor process*/
	sem_wait(mutex);
	(sh_var->visitor_cn)++;
	uint32_t cur_vis_num = sh_var->visitor_cn;
	sem_post(mutex);

	/*Visitor process notify its start*/
	print_act_2v(mutex, "%d: N %d: started\n", &sh_var->process_cn, &cur_vis_num);

	/*Every visitor comes to the park, for example, walking around, and goes to our attraction
	 * So this time he waits, and goes to the queue. As I see in the implementation example,
	 * every visitor takes place in queue and immediatly starts boarding*/
	usleep(arg->visitor_sleep);
	sem_wait(queue);

	print_act_2v(mutex, "%d: N %d: queue\n", &sh_var->process_cn, &cur_vis_num);
	print_act_2v(mutex, "%d: N %d: boarding\n", &sh_var->process_cn, &cur_vis_num);

	/*Step by step visitor boarding in to cart, while cart isn't full. As for me, 
	 * The best implementation of this - turnstile. When the cart gets full, so number of
	 * boarded visitor equals its capacity(if less, we don't expect more visitors today)*/

	/*Here every visitor increment turnstile counter, it must be in mutex*/
	sem_post(boarding_turnstile);

	/*Visitors wait signal from cart to start leaving*/
	sem_wait(visitor_leaving);

	print_act_2v(mutex, "%d: N %d: leaving\n", &sh_var->process_cn, &cur_vis_num);
	sem_post(leaving_turnstile);

}
