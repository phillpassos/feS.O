#ifndef _SYSCALL
#define _SYSCALL

#include "Idt.h"

#define INFO_PROCESSOS 	1
#define INFO_MENSAGENS 	2
#define INFO_MEMORIA 	3
#define INFO_VFS 		4

class Syscall
{
	
   public :
   
   unsigned int num;
   
   void inicializar(Idt idt);
   void chamar_systemcall(struct regs *r);
   void sbrk(struct regs *r);
   void obter_info_kernel(struct regs *r);
   void enviar_msg(struct regs *r);
   void enviar_msg_pid(struct regs *r);
   void receber_msg(struct regs *r);
   void escutar_porta(struct regs *r);
   void abrir_arquivo(struct regs *r);
   void ler_arquivo(struct regs *r);
   void escrever_arquivo(struct regs *r);
   void buscar_arquivo(struct regs *r);
   void excluir_arquivo(struct regs *r);
   void fechar_arquivo(struct regs *r);
   void executar(struct regs *r);
   void sair(struct regs *r);
   void finalizar_proceso(struct regs *r);
   void criar_thread(struct regs *r);
   void obter_pid(struct regs *r);
   void obter_info_arquivo(struct regs *r);
   void adicionar_no(struct regs *r);
   void montar_no(struct regs *r);
   void remover_no(struct regs *r);
};
 
extern Syscall syscall;

#endif