/*struct of argument's variables*/
typedef struct{ 
	uint8_t card; 
	uint16_t visitor; 
	uint8_t card_capacity; 
	uint16_t card_time; 
	uint16_t visitor_time; 
	uint8_t safe_interval; 

} arguments; 

/*constructor of argument's structure*/
arguments* arg_strc_ctor(void);

/*constructor of shared variables*/
shared_variables *sh_var_ctor(void);

/*destructor of arguments' structure*/
void arg_strc_dtor(arguments *arg);

/*destructor of shared variables' structure*/
void sh_var_dtor(shared_variables *sh_var);

/*function reading and assetring values of arguments*/
uint8_t arguments_reading(int argc, char **argv, arguments *arg);
