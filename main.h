
/*struct of argument's variables*/
typedef struct{ 
	uint8_t vozik; 
	uint16_t navstevnik; 
	uint8_t kapacita; 
	uint16_t vozik_time; 
	uint16_t navstevnik_time; 
	uint8_t odstup; 

} arguments; 

/*constructor of argument's structure*/
arguments* arg_strc_ctor(void);

/*function reading and assetring values of arguments*/
uint8_t arguments_reading(int argc, char **argv, arguments *arg);
