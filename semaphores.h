typedef struct{
	uint16_t* visitor_num;
	uint8_t* visitor_ts_count;
	uint8_t* card_num;

} shared_variables;

void dispatcher_semaphore(arguments *arg);
void visitor_semaphore(arguments *arg, shared_variables *sh_var);
void card_semaphore(arguments *arg, shared_variables *sh_var));
