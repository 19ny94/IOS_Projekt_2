/* @author Oleksandr Andreichenko (xandreo00)*/

#include<stdio.h>
#include<stdarg.h>
#include<stdint.h>
#include<unistd.h>
#include<stdlib.h>
#include"semaphores.h"

#define SEMAPHORE_CN 9

	/*---Init semaphore structure ---
	 * Every semaphore in structure has his name,
	 * name in "" and init value
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
		 * second - O_CREAT - when this flag specified, function create semaphore, if it doesn't already exist
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
		/*sem_close return -1 if it unsuccess*/
		if(sem_close(*sem_ct_arr[sem_idx].sem_arr) != 0){return 1;}
	}
	return 0;
}

void print_act(sem_t* mutex, char *action_txt, uint32_t *variable){
	sem_wait(mutex);
	(*variable)++;
	fprintf(stdout, action_txt, *variable);
	sem_post(mutex);
}

void print_act_2v(sem_t* mutex, char *action_txt, uint32_t *variable, uint32_t *variable_2){
	sem_wait(mutex);
	(*variable)++;
	fprintf(stdout, action_txt, *variable, *variable_2);
	sem_post(mutex);
}

void dispatcher_semaphore(arguments *arg, shared_variables *sh_var){
	
	/*Dispatcher process notify its' start*/
	print_act(mutex, "%d D: started\n", sh_var->process_cn);

	while(1){
		if(*sh_var->visitor_gl_cn == 0){
		break;
		}
		/*Dispatcher send signal next cart to start*/
		print_act(mutex, "%d D: next cart\n", sh_var->process_cn);
		sem_post(dis_next_cart);
		
		/*Dispatcher waits when cart leave boarding station.
		 * After waits safe time interval and send signal to next cart*/
		sem_wait(boarding_signal);
		usleep(arg->safe_interval);
	
		/*Cart understand if it go not full
		 * So in this case, dispatcher make signal, that
		 * if some cart is taking a ride now - 
		 * they finish and close. For other 
		 * cart, which currently free,
		 * dispatcher just send signal*/
		if(*sh_var->finish_flag != 0){
		break;
		}
	}
	
	print_act(mutex, "%d D: closing\n", sh_var->process_cn);
	for(uint8_t cart_to_lv = 0; cart_to_lv < arg->cart_gl_cn; cart_to_lv++){
	sem_post(dis_next_cart);
	}
}

void cart_semaphore(arguments *arg, shared_variables *sh_var){

	/*Assign number to current cart process*/
	sem_wait(mutex);
	(*sh_var->cart_cn)++;
	uint32_t cur_cart_num = *sh_var->cart_cn;
	sem_post(mutex);

	/*Cart process notify its' start*/
	print_act_2v(mutex, "%d V %d: started\n", sh_var->process_cn, &cur_cart_num);

	while(1){

	/*Cart waits dispatcher signal to start boarding*/
	sem_wait(dis_next_cart);

	/* If there ain't any more visitors -> finish
	 * I did it here, because all semaphores, even if they
	 * just started by dispatcher to just close it self*/
	if(*sh_var->visitor_gl_cn == 0){
	break;
	}

	/*As i got, boarding and leaving stations are critical sections in our case
	 * So, only one cart can currently be at either station*/
	sem_wait(boarding_station);
	
	print_act_2v(mutex, "%d V %d: boarding started\n", sh_var->process_cn, &cur_cart_num);

	uint32_t cur_vis_on_board = 0;

	/*Exactly when certain number of visitors boarded, 
	 * this certain number we take away from visitor global counter
	 *
	 * So, for example, if current cart take last visitors on a board,
	 * so it's gotta be last cart for today
	 * Next one just finish, before it started*/
	sem_wait(mutex);
	if(*sh_var->visitor_gl_cn >= arg->cart_capacity){
		cur_vis_on_board = arg->cart_capacity;
	}
	else{
		cur_vis_on_board = *sh_var->visitor_gl_cn;
		*sh_var->finish_flag = 1;
	}
	*sh_var->visitor_gl_cn -= cur_vis_on_board;
	sem_post(mutex);

	/*Cart sends signal to n-visitors, where n - number of visitor, which can 
	 * now board*/
	for(uint8_t vis_sig = 0; vis_sig < cur_vis_on_board; vis_sig++){
	sem_post(queue);
	sem_wait(boarding_turnstile);
	}

	/*Cart free critical section and send signal to next one*/
	print_act_2v(mutex, "%d V %d: boarding complete\n", sh_var->process_cn, &cur_cart_num);

	/*Cart free station for next cart*/
	sem_post(boarding_station);

	/*Also send signal to dispatcher that cart has already boarded*/
	sem_post(boarding_signal);

	/*We're leaving, fasten your seat belts*/
	usleep(arg->cart_sleep);

	/*Cart comes to leaving station*/
	sem_wait(leaving_station);
	
	print_act_2v(mutex, "%d V %d: leaving started\n", sh_var->process_cn, &cur_cart_num);

	/*Cart send a instruction to visitors to leave*/
	for(uint8_t vis_sig = 0; vis_sig < cur_vis_on_board; vis_sig++){
		sem_post(visitor_leaving);
		sem_wait(leaving_turnstile);
	}
	print_act_2v(mutex, "%d V %d: leaving complete\n", sh_var->process_cn, &cur_cart_num);
	
	/*Cart free leaving station*/
	sem_post(leaving_station);
	}

	print_act_2v(mutex, "%d V %d: closed\n", sh_var->process_cn, &cur_cart_num);

}

void visitor_semaphore(arguments *arg, shared_variables *sh_var){

	/*Assign number to current visitor process*/
	sem_wait(mutex);
	(*sh_var->visitor_cn)++;
	uint32_t cur_vis_num = *sh_var->visitor_cn;
	sem_post(mutex);

	/*Visitor process notify its' start*/
	print_act_2v(mutex, "%d N %d: started\n", sh_var->process_cn, &cur_vis_num);

	/*Every visitor comes to park, for example, walking around, and go to our atraction
	 * So this time he waits, and go to queue. As i see in implementation example,
	 * every visitor take place in queue and immidiatly starts boarding*/
	usleep(arg->visitor_sleep);
	sem_wait(queue);

	print_act_2v(mutex, "%d N %d: queue\n", sh_var->process_cn, &cur_vis_num);
	print_act_2v(mutex, "%d N %d: boarding\n", sh_var->process_cn, &cur_vis_num);

	/*Step by step visitor boarding in to cart, while cart isn't full. As for me, 
	 * the best implementation of this - turnstile. When cart get full, so number of
	 * boarded visitor equal it's capacity(if less, we not expected more visitors today)*/

	/*Here every visitor increment turnstile counter, it's must be in mutex*/
	sem_post(boarding_turnstile);

	/*Visitors wait signal form cart to start leaving*/
	sem_wait(visitor_leaving);

	print_act_2v(mutex, "%d N %d: leaving\n", sh_var->process_cn, &cur_vis_num);
	sem_post(leaving_turnstile);

}
