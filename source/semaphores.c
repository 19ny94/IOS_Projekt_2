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
	sem_open("/leaving_turnstile", O_CREAT ,0644, 0);
	
	boarding_signal =
	sem_open("/boarding_signal", O_CREAT ,0644, 0);



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
	while(1){

			/*Mutex here because count*/
		sem_wait(mutex);
		(*sh_var->cart_gl_cn)--;
		(*sh_var->process_cn)++;
		fprintf(stdout, "%d: D: next cart\n", *sh_var->process_cn);
		sem_post(dis_next_card);
		sem_post(mutex);

		/*Dispatcher waits safe from last cart*/
		usleep(arg->safe_interval);

		/*Waits for signal from cart to continue loop*/
		sem_wait(boarding_signal);
		if(*sh_var->finish_flag != 0){
		break;
		}

	}
	if(*sh_var->finish_flag != 0){
	fprintf(stdout, "%d: D: closing\n", *sh_var->process_cn);
	}
	
}

void cart_semaphore(arguments *arg, shared_variables *sh_var){
	
	/*Process cart starts*/
	sem_wait(mutex);
	(*sh_var->process_cn)++;
	uint8_t cur_cart_num = *sh_var->cart_cn;
	fprintf(stdout, "%d: V %d: started\n", *sh_var->process_cn, cur_cart_num);
	sem_post(mutex);

	/*Current cart increment his number, in mutex of course*/
	sem_wait(mutex);
	(*sh_var->cart_cn)++;
	sem_post(mutex);

	while(*sh_var->visitor_gl_cn != 0){
	/*Waits signal from dispatcher to start boarding*/
	sem_wait(dis_next_card);

	/*Now cart come to boarding station, what is critical section
	 * only one cart at the moment can boarding or leaving*/

	sem_wait(mutex);
	if(*sh_var->visitor_gl_cn >= arg->cart_capacity){
		*sh_var->cur_vis_on_board = arg->cart_capacity;
	}
	else{
		*sh_var->cur_vis_on_board = *sh_var->visitor_gl_cn;
		*sh_var->finish_flag = 1;
	}

	sem_post(mutex);

	sem_wait(boarding_station);


	/*When cart arrivied at the boarding station - send signal to visitors
	* Cart send signal to n-visitors, where n - cart_capacity*/


	sem_wait(mutex);
	(*sh_var->process_cn)++;
	fprintf(stdout, "%d: V %d: boarding started\n", *sh_var->process_cn, cur_cart_num);
	sem_post(mutex);

	for(uint8_t visitors_one_ride = 0; visitors_one_ride < *sh_var->cur_vis_on_board; visitors_one_ride++){
		sem_post(visitor_boarding);
	}

	sem_wait(boarding_turnstile);

	sem_wait(mutex);
	
	(*sh_var->process_cn)++;
	fprintf(stdout, "%d: V %d: boarding complete\n", *sh_var->process_cn, cur_cart_num);
	sem_post(mutex);
	

	sem_wait(mutex);
	*sh_var->visitor_ts_cn = 0;
	sem_post(mutex);
	
	/*When current cart finished - send signal to next one*/
	sem_post(boarding_station);


	/*Now we take a ride. Wait some time(will be imlement later)
	 * Now time to finish our ride*/
	usleep(arg->cart_sleep);

	/*Again, leaving platfrom is critical section*/
	sem_wait(leaving_station);
	
	/*Send signal to n-visitors to leave*/
	sem_wait(mutex);
	(*sh_var->process_cn)++;
	fprintf(stdout, "%d: V %d: leaving started\n", *sh_var->process_cn, cur_cart_num);
	sem_post(mutex);

	for(uint8_t visitors_one_ride = *sh_var->cur_vis_on_board; visitors_one_ride > 0; visitors_one_ride--){
		sem_post(visitor_leaving);
	}
	
	sem_wait(leaving_turnstile);

	sem_wait(mutex);
	(*sh_var->process_cn)++;
	fprintf(stdout, "%d: V %d: leaving complete\n", *sh_var->process_cn, cur_cart_num);
	*sh_var->visitor_ts_lv = 0;
	sem_post(mutex);
	
	/*When ride ends, that obvious that visitor counter gotta dicrement
	 *Because on start this function i init current visitors' count, who take a ride currently
	 *That sure that i zero counter in some future*/
	sem_wait(mutex);
	*sh_var->visitor_gl_cn -= *sh_var->cur_vis_on_board;
	if(*sh_var->visitor_gl_cn == 0){
		*sh_var->finish_flag = 1;
	}
	sem_post(mutex);

	sem_post(leaving_station);
	sem_post(boarding_signal);

	sem_wait(mutex);
	(*sh_var->cart_gl_cn)++;
	sem_post(mutex);
	}
}

void visitor_semaphore(arguments *arg, shared_variables *sh_var){

	/*Every visitor increments his number*/
	sem_wait(mutex);
	(*sh_var->visitor_cn)++;
	uint16_t cur_vis_num = *sh_var->visitor_cn;
	sem_post(mutex);

	/*Process visitor starts*/
	sem_wait(mutex);
	(*sh_var->process_cn)++;
	fprintf(stdout, "%d: N %d: started\n", *sh_var->process_cn, cur_vis_num);
	sem_post(mutex);

	usleep(arg->visitor_sleep);

	/*In the implementation reference, all visitors when they receive signal from
	 * cart go to queue and immidiatly they start boarding, so, i think that
	 * imlemented in semaphore*/
	sem_wait(visitor_boarding);

	sem_wait(mutex);
	(*sh_var->process_cn)++;
	fprintf(stdout, "%d: N %d: queue\n", *sh_var->process_cn, cur_vis_num);
	(*sh_var->process_cn)++;
	fprintf(stdout, "%d: N %d: boarding\n", *sh_var->process_cn, cur_vis_num);
	sem_post(mutex);

	sem_wait(mutex);
	(*sh_var->visitor_ts_cn)++;
	fprintf(stdout, "DEBUG2: after increment ts_cn=%d cur=%d\n",
        *sh_var->visitor_ts_cn, *sh_var->cur_vis_on_board);

	if(*sh_var->visitor_ts_cn == *sh_var->cur_vis_on_board){
	fprintf(stdout, "DEBUG: visitor_ts_cn=%d cur_vis_on_board=%d\n", 
        *sh_var->visitor_ts_cn, *sh_var->cur_vis_on_board);
		sem_post(boarding_turnstile);
	}
	sem_post(mutex);

	sem_wait(visitor_leaving);

	sem_wait(mutex);
	(*sh_var->process_cn)++;
	fprintf(stdout, "%d: N %d: leaving\n", *sh_var->process_cn, cur_vis_num);
	sem_post(mutex);

	sem_wait(mutex);
	(*sh_var->visitor_ts_lv)++;

	if(*sh_var->visitor_ts_lv == *sh_var->cur_vis_on_board){
		sem_post(leaving_turnstile);
	}
	sem_post(mutex);
}
