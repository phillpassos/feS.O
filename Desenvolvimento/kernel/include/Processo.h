#ifndef _PROC
#define _PROC

#include "Lista.h"
#include "Avl.h"
#include "Idt.h"
#include "Mensagem.h"
#include "Multiprocessamento.h"

#define QTD_FRAMES_PILHA 10

//constantes da gerência de processos
const int PROC_SUCESSO  = 1;
const int PROC_FORMATO_INVALIDO = 2;
const int PROC_SEM_MEMORIA = 3;

class Thread : public Recurso
{
   public :
   
   Lista<Mensagem> mensagens; 			  // lista de mensagens 
   int id_processo;					      // processo ao qual a thread pertence		
   regs registradores;                    // registradores que armazenam o estado daa thread 
   
   unsigned int pid;               		  // id da thread
   unsigned int pid_pai;               	  // id do pai
   unsigned int tipo;            		  // tipo do processo (PROCESSO COMUM, DRIVER, IDLE)
   unsigned int estado;            		  // estado do processo (PRONTO, ESPERANDO, FINALIZADO)
   unsigned int pdir;			   		  // endereço físico da Page Directory 
   unsigned int entrada;		   		  // ponto de entrada da thread
     
   void iniciar(unsigned int);
   void alternar_para(struct regs *);
   void adicionar_mensagem(Mensagem msg);
   void destruir();
   Mensagem& obter_mensagem(int pid, int porta);
   
};

class Processo: public Recurso
{
  
   public :
   
   static const int NAO_INICIADO = 0;
   static const int PRONTO 		 = 1;
   static const int ESPERANDO 	 = 2;
   static const int EXECUTANDO 	 = 3;
   static const int FINALIZADO 	 = 4;
   static const int DRIVER	 	 = 0;
   static const int USUARIO	 	 = 1;
   static const int IDLE	 	 = 2;
   
   char nome[15];
   Thread thread_inicial;

   Lista<unsigned int> enderecos_fisicos; // lista de frames ocupados pelo processo
   Lista<unsigned int> threads;			  // lista de threads do processo
   regs registradores;                    // registradores que armazenam o estado do processo 
   
   unsigned int pid;               		  // id do processo (é o igual ao id da thread principal)
   unsigned int pid_pai;               	  // id do pai
   unsigned int tipo;            		  // tipo do processo
   unsigned int estado;            		  // estado do processo
   unsigned int end_virtual;       		  // último endereço virtual ocupado pelo processo
   unsigned int pdir;			   		  // endereço da Page Directory
   unsigned int heap;					  // heap do processo							
   unsigned int ult_end_ret;			  // último endereço a ser utilizado no heap
   unsigned int proxima_pilha;			  // endereço do começo da pilha a ser atribuída à próxima thread criada
   unsigned int entrada;				  // ponto de entrada do processo(é o ponto de entrada da thread principal)	
    
   int criar_processo(unsigned int p, unsigned char * nome, unsigned char * imagem, int ptipo, int argc, char * args[]);
   Thread criar_thread(int id, int ponto_entrada, int pilha);
   int obter_nova_pilha();
   void criar_espaco_enderecamento(unsigned int *);
   void aumentar_heap(unsigned int * end, unsigned int tam);
   void destruir();
   
};

class Escalonador : public Recurso
{
	unsigned int prox_pid;                    //próximo valor de PID a ser atribuído a uma thread
	
	unsigned int pid_idle;					  //pid do processo IDLE	
	
	//Lista<unsigned int> threads_a_excluir;  //lista de PIDs de threads a serem removidas
    Lista<unsigned int> threads_prontas;      //lista de PIDs de threads prontas
	Lista<unsigned int> drivers_prontos;      //lista de PIDs de drivers prontos
	
	Arvore<Processo> processos; 			  //estrutura que armazena todos os processos
	Arvore<Thread> threads; 				  //estrutura que armazena todas as threads
	
	public :
	volatile int escalonamento_ativo; 		  //define se o escalonamento está ou não ativo
	unsigned int thread_em_execucao[16];      //thread que está ocupando a CPU no momento
	
	void inicializar();
	void executar_prox_thread(struct regs *r);
	int adicionar_processo(unsigned char * nome, unsigned char * imagem, int tipo, int argc, char * args[]);
	int adicionar_thread(Processo &p, int ponto_entrada);
	int obter_pid_em_execucao();
	int driver_em_execucao();
	void adicionar_idle(unsigned char * imagem);
	void adicionar_thread_pronta(Thread p);
	void eliminar_processo(int pid);
	void eliminar_processo(Processo &p);
	void eliminar_thread(Thread &t);
	void notificar_exception(unsigned int, struct regs *r);
	void obter_info(unsigned char * arq);
	void acordar_thread(int pid);
	void colocar_thread_atual_em_espera();
	int adicionar_mensagem(int pid, Mensagem m);
	Mensagem& obter_mensagem(int pid, int porta, int pid_responsavel);
	Processo& obter_processo(unsigned int);
	Thread& obter_thread(unsigned int);
	
};
 
extern volatile Escalonador escalonador;



#endif


