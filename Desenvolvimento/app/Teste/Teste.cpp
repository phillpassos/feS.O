#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Lista.h"
#include "include/Io.h"
#include "include/Sistimer.h"
#include "include/Ide.h"

const int TAM = 1024;

int A[TAM];
int B[TAM];
int C[TAM];

struct TBloco
{
	int pid;
	int ini;
	int fim;
};

void calcular()
{
	int pid = obter_pid();
	TBloco b;
	b.pid = pid;
	char msg[100];
	
	memcpy(msg, (char *)&b, sizeof(TBloco));
	enviar_receber_msg(1001, msg);
	memcpy((char *)&b, msg, sizeof(TBloco));
	
	for(int i = b.ini; i < b.fim; i++)
	{
		for(int j = 0; j < 10000000; j++);
		C[i] = B[i] + A[i];
	}
	
	enviar_msg(1001, msg);

	sair();
}

	
//------------processo para testes
main(int argc, char ** args)
{
	unsigned char tecla, msg[100];
	unsigned int cont = 0;
	int pid = obter_pid();
	
	
	
	Relogio r;
	
	int t0;
	obter_relogio(r);
	t0 = r.ms_dia;
	
	if(argc == 1)
	{
		escutar_porta(1001);
		int qtd_threads =  atoi(args[0]);
	
		for(int i =0; i < TAM; i++)
			A[i] = B[i] = i;
	
		printf("%d\n", qtd_threads);
		
		for(int i =0;i<qtd_threads; i++)
		{
			int t = criar_thread(calcular);
			receber_msg(msg,MSG_QLQR_PORTA);
			
			TBloco b;
			b.ini = i*TAM/qtd_threads;
			b.fim = b.ini + TAM/qtd_threads; 
	
			printf("recebeu %d\n", t);
			memcpy(msg, (char *)&b, sizeof(TBloco));
			
			enviar_msg_pid(t, msg);
			
		}
		
		
		for(int i =0;i<qtd_threads; i++)
			receber_msg(msg, MSG_QLQR_PORTA);
	
	
		obter_relogio(r);
		int t1 = r.ms_dia;
		
		int desc = abrir("/resultado",'a');
		
		for(int i = 0; i < TAM; i++)
		{
			escrever(desc, itoa(C[i],10), strlen(itoa(C[i],10)));
			escrever(desc, "\n", 1);
		}
		
		escrever(desc, "\n", 1);
		escrever(desc, itoa(t1-t0,10), strlen(itoa(t1-t0,10)));
		fechar(desc);

		sair();
	}

	for(;;)
	{
		for(unsigned int i =0; i < 100000000; i++);
		//asm volatile("mov %%ebx, %0"::"r"(pid));
	 	//printf("%d\n", pid);
		posicionar_cursor(72,0);
		esperar(200);
		Relogio r;
		obter_relogio(r);
		posicionar_cursor(0,1);
		
		if(r.hora < 10)
		{
			printf("0");
		}
		
		printf("%d:",r.hora);
		
		if(r.min < 10)
		{
			printf("0");
		}
		
		printf("%d:",r.min);
		
		if(r.seg < 10)
		{
			printf("0");
		}
		
		printf("%d",r.seg);
		
	}
	printf("Pressione qualquer tecla para finalizar\n");
	getchar(&tecla);
	sair();
	
}//fim da main

