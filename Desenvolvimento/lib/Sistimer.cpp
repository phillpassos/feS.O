#include "include/Sistimer.h"
#include "include/Sistema.h"
#include "include/Util.h"

static unsigned long int proximo = 1;//usado pelo rand

void esperar(int milisegundos)
{
	unsigned char msg[100];
	
	//criando bloco de comunicação com o servidor
	Bloco_Timer b;
	b.codigo = 1;
	b.pid = obter_pid();
	b.param1 = milisegundos;
	memcpy(msg, (unsigned char *) &b, sizeof(Bloco_Timer));
	
	//enviando mensagem para porta 81 (timer)
	enviar_receber_msg(81, msg);	
}

void obter_relogio(Relogio &r)
{
	unsigned char msg[100];
	
	//criando bloco de comunicação com o servidor
	Bloco_Timer b;
	b.codigo = 2;
	b.pid = obter_pid();
	memcpy(msg, (unsigned char *) &b, sizeof(Bloco_Timer));
	
	//enviando mensagem para porta 81 (timer)
	enviar_receber_msg(81, msg);
	
    memcpy((unsigned char *) &r, msg, sizeof(Relogio));
}

double pit()
{
	double retorno;
	unsigned char msg[100];
	
	//criando bloco de comunicação com o servidor
	Bloco_Timer b;
	b.codigo = 3;
	b.pid = obter_pid();
	memcpy(msg, (unsigned char *) &b, sizeof(Bloco_Timer));
	
	//enviando mensagem para porta 81 (timer)
	enviar_receber_msg(81, msg);
	
    memcpy((unsigned char *) &retorno, msg, sizeof(double));
	return retorno;
}
 
int rand( void ) // RAND_MAX assumed to be 32767
{
    proximo = proximo * 1103515245 + 12345;
    return (unsigned int)(proximo / 65536)/* % 32768*/;//< sem mod, valor max = 2^16
}
 
void srand( unsigned int semente )
{
    proximo = semente;
}