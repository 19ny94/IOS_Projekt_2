#include<semaphore.h>
#include"main.h"

/*Stucture of shared variables, opens by mmap()*/
typedef struct{
	/*Current visitor number, every visitor read value 
	 * into his local variable, after this do increment*/
	uint32_t visitor_cn;

	/*How many visitors we expect during park open
	 *Cart */
	uint32_t visitor_gl_cn;

	/*Current cart number*/
	uint32_t cart_cn;

	/*Rather for case, when number of left visitors less then cart capacity*/
	uint32_t cur_vis_on_board;

	/*Log/process ordinal number*/
	uint32_t process_cn;

	/*After assembler experience this seems as a good idea
	 * Finish flag - we are about to finish, but not exatly now
	 * Cart sets flag, when it understands, that one going to be last ride*/
	uint8_t finish_flag;

} shared_variables;

typedef struct{

	sem_t*  *sem_arr;
	const char * sem_name;
	uint8_t init_value;

} semaphore_arr;

/*Opened all semaphores*/
uint8_t all_semaphores_open(void);

/*Close all semaphores*/
uint8_t all_semaphores_close(void);

/*Implemented 3 types of processes*/
void dispatcher_semaphore(arguments *arg, shared_variables *sh_var);
void cart_semaphore(arguments *arg, shared_variables *sh_var);
void visitor_semaphore(arguments *arg, shared_variables *sh_var);

/*Allocate structure of shared variables by mmap()*/
shared_variables* sh_var_ctor(arguments *arg);

/*Free structure by munmap()*/
void sh_var_dtor(shared_variables *sh_var);

/*Function do fork() in loop for every type of processes*/
void process_branching(arguments *arg, shared_variables *sh_var);
