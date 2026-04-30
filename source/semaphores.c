#include<stdio.h>
#include<stdint.h>
#include<unistd.h>
#include"semaphores.h"

#define SEMAPHORE_CN 9





/*Open all semaphores*/
void all_semaphores_open(void){


	dis_next_cart =
	sem_open("/dis_next_cart", O_CREAT ,0644, 0);
	
	boarding_station =
	sem_open("/boarding_station", O_CREAT ,0644, 1);

	mutex =
	sem_open("/mutex", O_CREAT ,0644, 1);

	queue =
	sem_open("/queue", O_CREAT ,0644, 0);

	leaving_station =
	sem_open("/leaving_station", O_CREAT ,0644, 1);

	visitor_leaving =
	sem_open("/visitor_leaving", O_CREAT ,0644, 0);

	boarding_turnstile =
	sem_open("/boarding_turnstile", O_CREAT ,0644, 0);

	leaving_turnstile =
	sem_open("/leaving_turnstile", O_CREAT ,0644, 0);
	
	boarding_signal =
	sem_open("/boarding_signal", O_CREAT ,0644, 0);
}

/*Function close and unlink all opened semaphores*/
void all_semaphores_close(void){

	char* sem_name[SEMAPHORE_CN] = {"/dis_next_cart",
	"/boarding_station", "/mutex", "/queue",
	"/leaving_station", "/visitor_leaving", "/boarding_turnstile",
	"/char* leaving_turnstile", "/boarding_signal"};

	sem_t* sem_arr[SEMAPHORE_CN] = {dis_next_cart,
	boarding_station, mutex, queue,
	leaving_station, visitor_leaving, boarding_turnstile,
	leaving_turnstile,boarding_signal};

	for(uint8_t sem_i = 0; sem_i < SEMAPHORE_CN; sem_i++){
	sem_unlink(sem_name[sem_i]);
	sem_close(sem_arr[sem_i]);
	}
}

void print_act(sem_t* mutex, char *action_txt, uint16_t *variable){
	sem_wait(mutex);
	(*variable)++;
	fprintf(stdout, action_txt, *variable);
	sem_post(mutex);
}

void print_act_2v(sem_t* mutex, char *action_txt, uint16_t *variable, uint16_t *variable_2){
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
		
		sem_wait(boarding_signal);
		/*Next one he'll send after safe time*/
		usleep(arg->safe_interval);
	
		/*Cart understand if it's go not full
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
	uint16_t cur_cart_num = *sh_var->cart_cn;
	(*sh_var->cart_gl_cn)--;
	sem_post(mutex);


	/*Cart process notify its' start*/
	print_act_2v(mutex, "%d V %d: started\n", sh_var->process_cn, &cur_cart_num);

	while(1){

	/*Cart waits dispatcher signal to start boarding*/
	sem_wait(dis_next_cart);

	if(*sh_var->visitor_gl_cn == 0){
	break;
	}

	/*As i got, boarding and leaving stations are critical sections in our case
	 * So, only one cart can currently be at either station*/
	sem_wait(boarding_station);


	
	print_act_2v(mutex, "%d V %d: boarding started\n", sh_var->process_cn, &cur_cart_num);

	uint8_t cur_vis_on_board = 0;
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


	/*Now really important moment. Because somethimes we need to ride not full cart 
	 * we need to calculate, how many visitors we expected for next ride*/

	/*Cart free critical section and send signal to next one*/
	print_act_2v(mutex, "%d V %d: boarding complete\n", sh_var->process_cn, &cur_cart_num);

	sem_post(boarding_station);


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
	
	sem_wait(mutex);
	(*sh_var->cart_gl_cn)++;	
	sem_post(mutex);

	}

	print_act_2v(mutex, "%d V %d: closed\n", sh_var->process_cn, &cur_cart_num);

}

void visitor_semaphore(arguments *arg, shared_variables *sh_var){

	/*Assign number to current visitor process*/
	sem_wait(mutex);
	(*sh_var->visitor_cn)++;
	uint16_t cur_vis_num = *sh_var->visitor_cn;
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
