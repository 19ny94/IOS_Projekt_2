/*struct of argument's variables*/
typedef struct{ 
	uint8_t cart_gl_cn; 
	uint16_t visitor_gl_cn; 
	uint8_t cart_capacity; 
	uint16_t cart_time; 
	uint16_t visitor_time; 
	uint8_t safe_interval; 
	uint16_t visitor_sleep;
	uint16_t cart_sleep;
} arguments; 

/**/
arguments* arg_strc_ctor(void);

void arg_strc_dtor(arguments *arg);

uint8_t arguments_reading(int argc, char **argv, arguments *arg);
