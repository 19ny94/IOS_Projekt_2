#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>
#include"semaphores.h"

void all_semaphores_open(void){

	sem_unlink("/dis_next_card");
	sem_unlink("/boarding_station");
	sem_unlink("/mutex");
	sem_unlink("/visitor_boarding");
	sem_unlink("/leaving_station");
	sem_unlink("/visitor_leaving");
	sem_unlink("/boarding_turnstile");
	sem_unlink("/leaving_turnstile");

	dis_next_card =
	sem_open("/dis_next_card", O_CREAT ,0644, 0);
	
	boarding_station =
	sem_open("/boarding_station", O_CREAT ,0644, 1);

	mutex =
	sem_open("/mutex", O_CREAT ,0644, 1);

	visitor_boarding =
	sem_open("/visitor_boarding", O_CREAT ,0644, 0);
	
	leaving_station =
	sem_open("/leaving_station", O_CREAT ,0644, 1);

	visitor_leaving =
	sem_open("/visitor_leaving", O_CREAT ,0644, 0);

	boarding_turnstile =
	sem_open("/boarding_turnstile", O_CREAT ,0644, 0);

	leaving_turnstile =
	sem_open("/leaving_turnstile ", O_CREAT ,0644, 0);
}

void close_all_semaphores(void){
	sem_close(dis_next_card );
	sem_close(boarding_station );
	sem_close(mutex);
	sem_close(visitor_boarding);
	sem_close(leaving_station );
	sem_close(visitor_leaving );
	sem_close(boarding_turnstile );
	sem_close(leaving_turnstile );

}

void dispatcher_semaphore(arguments *arg, shared_variables *sh_var){
	/*Process dispatcher starts*/
	fprintf(stdout, "%d: D: started\n", *sh_var->process_cn);

	/*Call next card to start boarding*/
	while(*sh_var->finish_flag == 0){
		sem_wait(mutex);
		if(*sh_var->cart_gl_cn > 0){
		(*sh_var->cart_gl_cn)--;
		fprintf(stdout, "%d: D: next cart\n", *sh_var->process_cn);
		sem_post(dis_next_card);
		}
		sem_post(mutex);
		usleep(arg->safe_interval);
	}
	if(*sh_var->finish_flag != 0){
	fprintf(stdout, "%d: D: closing\n", *sh_var->process_cn);
	}
	
}

void cart_semaphore(arguments *arg, shared_variables *sh_var){
	
	/*Process cart starts*/
	fprintf(stdout, "%d: V %d: started\n", *sh_var->process_cn, *sh_var->cart_cn);

	/*Current cart increment his number, in mutex of course*/
	sem_wait(mutex);
	(*sh_var->cart_cn)++;
	sem_post(mutex);

	while(*sh_var->finish_flag == 0){
	/*Waits signal from dispatcher to start boarding*/
	sem_wait(dis_next_card);

	/*Now cart come to boarding station, what is critical section
	 * only one cart at the moment can boarding or leaving*/
	sem_wait(boarding_station);

	sem_wait(mutex);
	if(*sh_var->visitor_gl_cn >= arg->cart_capacity){
		*sh_var->cur_vis_on_board = arg->cart_capacity;
	}
	else{
		*sh_var->cur_vis_on_board = *sh_var->visitor_gl_cn;
	}

	sem_post(mutex);

	/*When cart arrivied at the boarding station - send signal to visitors
	* Cart send signal to n-visitors, where n - cart_capacity*/

	fprintf(stdout, "%d: V %d: boarding started\n", *sh_var->process_cn, *sh_var->cart_cn);

	for(uint8_t visitors_one_ride = 0; visitors_one_ride < *sh_var->cur_vis_on_board; visitors_one_ride++){
		sem_post(visitor_boarding);
	}

	sem_wait(boarding_turnstile);

	fprintf(stdout, "%d: V %d: boarding complete\n", *sh_var->process_cn, *sh_var->cart_cn);
	/*When current cart finished - send signal to next one*/
	sem_post(boarding_station);

	/*Now we take a ride. Wait some time(will be imlement later)
	 * Now time to finish our ride*/

	/*Again, leaving platfrom is critical section*/
	sem_wait(leaving_station);
	
	/*Send signal to n-visitors to leave*/
	fprintf(stdout, "%d: V %d: leaving started\n", *sh_var->process_cn, *sh_var->cart_cn);

	for(uint8_t visitors_one_ride = *sh_var->cur_vis_on_board; visitors_one_ride > 0; visitors_one_ride--){
		sem_post(visitor_leaving);
	}
	
	sem_wait(leaving_turnstile);

	fprintf(stdout, "%d: V %d: leaving complete\n", *sh_var->process_cn, *sh_var->cart_cn);
	
	sem_wait(mutex);
	*sh_var->visitor_gl_cn -= *sh_var->cur_vis_on_board;
	if(*sh_var->visitor_gl_cn == 0){
		*sh_var->finish_flag = 1;
	}
	sem_post(mutex);

	sem_post(leaving_station);

	sem_wait(mutex);
	(*sh_var->cart_gl_cn)++;
	sem_post(mutex);
	}
}

void visitor_semaphore(arguments *arg, shared_variables *sh_var){

	/*Process visitor starts*/
	fprintf(stdout, "%d: N %d: started\n", *sh_var->process_cn, *sh_var->visitor_cn);

	/*Every visitor increments his number*/
	sem_wait(mutex);
	(*sh_var->visitor_cn)++;
	sem_post(mutex);

	/*In the implementation reference, all visitors when they receive signal from
	 * cart go to queue and immidiatly they start boarding, so, i think that
	 * imlemented in semaphore*/
	sem_wait(visitor_boarding);
	
	fprintf(stdout, "%d: N %d: queue\n", *sh_var->process_cn, *sh_var->visitor_cn);
	/*TODO: sleep for a while*/
	fprintf(stdout, "%d: N %d: boarding\n", *sh_var->process_cn, *sh_var->visitor_cn);

	sem_wait(mutex);
	(*sh_var->visitor_ts_cn)++;

	if(*sh_var->finish_flag == 0){
		if(*sh_var->visitor_ts_cn == arg->cart_capacity){
			sem_post(boarding_turnstile);
		}
	}
	else{
		if(*sh_var->visitor_ts_cn == *sh_var->visitor_gl_cn){
			sem_post(boarding_turnstile);
		}
	
	}
	sem_post(mutex);

	sem_wait(visitor_leaving);
	fprintf(stdout, "%d: N %d: leaving\n", *sh_var->process_cn, *sh_var->visitor_cn);

	sem_wait(mutex);
	(*sh_var->visitor_ts_cn)--;

	if(*sh_var->finish_flag == 0){
		if(*sh_var->visitor_ts_cn == 0){
			sem_post(leaving_turnstile);
		}
	}
	else{
		if(*sh_var->visitor_ts_cn == 0){
			sem_post(leaving_turnstile);
		}
	
	}
	sem_post(mutex);

}

