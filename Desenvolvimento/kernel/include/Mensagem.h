#ifndef _MENSAGEM
#define _MENSAGEM

#include "Avl.h"
#include "Irq.h"
#include "Lista.h"

#define PROC_CRIADO 0
#define PROC_DESTRUIDO 1

#define KERNEL_MSG 0
#define KERNEL_PORTA_LIM_INF 20
#define KERNEL_PORTA_LIM_SUP 40

#define KERNEL_PORTA_PROC_CRIADO 20
#define KERNEL_PORTA_PROC_DESTRUIDO 21
#define KERNEL_PORTA_ALOCADA 22
#define KERNEL_PORTA_DESALOCADA 23
#define KERNEL_PORTA_VFS 24
#define KERNEL_PORTA_EXCEPTION 25
	

//constantes sistema de troca de mensagems
const int MSG_COMUM = -1;
const int MSG_IRQ = -2;
const int MSG_EVT = -3;
const int MSG_QLQR_PORTA = -1;
const int NAO_HA_MSGS = 0;
const int MSG_SUCESSO = 1;
const int MSG_ERR_PROC_NAO_EXISTE = 5;
const int MSG_ERR_PORTA_NAO_ATIVA = 6;
const int MSG_ERR_PORTA_OCUPADA = 7;

#define TAM_MAX_MSG 100

//struct de eventos do sistema operacional
struct Evento
{
	char num;
	int param1;
	int param2;
	char dados[80];
};

struct Porta
{
	unsigned int numero;
	unsigned int pid_responsavel;
};

struct Mensagem
{
	int sinalizar_recebimento;
	int tipo;
	unsigned int pid_remetente;
	unsigned int porta_destino;
	unsigned char conteudo[TAM_MAX_MSG];
};

class Entregador : public Recurso
{
	Lista<int> kernel_listerners;
	Lista<int> kernel_listerners_evt_proc_criado;
	Lista<int> kernel_listerners_evt_proc_destruido;
	Lista<int> kernel_listerners_evt_porta_alocada;
	Lista<int> kernel_listerners_evt_porta_desalocada;
	Lista<int> kernel_listerners_evt_vfs;
	Lista<int> kernel_listerners_exception;
	Arvore<Porta> portas;
	Lista<int> portas_alocadas;	

	public:
	
	void inicializar(Irq irq);
	void notificar_irq(struct regs *r, int irq_num);
	void notificar_evento(char * dados, int evt_num);
	Porta& Entregador::obter_porta(int num);
	int adicionar_listerner(unsigned pid, unsigned porta);
	void remover_listerner(unsigned porta);
	int tratar_envio_pid(unsigned pid_origem, unsigned pid_destino, int tipo, unsigned char msg[TAM_MAX_MSG]);
	int tratar_envio(struct regs *r, unsigned pid, unsigned porta, int sinalizar, unsigned char msg[TAM_MAX_MSG]);
	int tratar_recebimento(struct regs *r, unsigned pid, unsigned char * msg, int porta);
	void obter_info(char * arq);
	
	
};

extern volatile Entregador entregador;

#endif