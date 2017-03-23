#ifndef _SISTIMER
#define _SISTIMER

struct Bloco_Timer
{
	int codigo;
	int pid;
	int param1;
	int param2;
};

struct Relogio
{
	unsigned int hora;
	unsigned int min;
	unsigned int seg;
	unsigned int ms;
	unsigned int ms_dia;
	int bcd;
	int hora24;
	
};

void esperar(int milisegundos);
void obter_relogio(Relogio&);
int rand();
int srand(unsigned int semente);
int pit();

#endif