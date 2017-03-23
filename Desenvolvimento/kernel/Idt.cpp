#include "include/Idt.h"
#include "include/Syscall.h"
#include "include/Memoria.h"
#include "include/Processo.h"
#include "include/Multiprocessamento.h"
#include "include/Mensagem.h"
#include "include/Util.h"


//Instância GLOBAL da IDT
Idt idt;

//ponteiro para a  IDT
volatile struct idt_ptr idtp;

//declaração de interrupções (funções contidas no start.asm)
extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();	
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();


registro_idt registros[256];  

//Configura a IDT 
void Idt::inicializar()
{
    //Obtendo o endereço da IDT para que a função em assembly  possa carregar a IDT  */
    idtp.limite = (sizeof (struct registro_idt) * 256) - 1;
    idtp.base   = (unsigned int)&registros;
	
    //Limpando a IDT, inicializando com zeros */
    memset((unsigned char*)registros, 0, sizeof(struct registro_idt) * 256);

    //Chamar função em assembly para configurar IDT
    carregar_idt();
	
	//Instala ISRs (rotinas para tratamento de exceção)
	instalar_isrs();
}

//Adicionar funções à IDT
void Idt::adicionar_registro(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
    //Dividindo o endereço base da rotina de tratamento de interrupção
    registros[num].base_low    = (base & 0xFFFF);
    registros[num].base_high   = (base >> 16) & 0xFFFF;

    //Definindo qual segmento será usado pela ISR (rotina de tratamento de interrupção)
    registros[num].seg         = sel;
    registros[num].sempre_zero = 0;
    registros[num].flags       = flags;
}

//Instala as ISRs (interrupt server routines) para as exceções
void Idt::instalar_isrs()
{
    adicionar_registro(0,  (unsigned)isr0, 0x08, 0x8E);
    adicionar_registro(1,  (unsigned)isr1, 0x08, 0x8E);
    adicionar_registro(2,  (unsigned)isr2, 0x08, 0x8E);
    adicionar_registro(3,  (unsigned)isr3,  0x08, 0x8E);
    adicionar_registro(4,  (unsigned)isr4,  0x08, 0x8E);
    adicionar_registro(5,  (unsigned)isr5,  0x08, 0x8E);
    adicionar_registro(6,  (unsigned)isr6,  0x08, 0x8E);
    adicionar_registro(7,  (unsigned)isr7,  0x08, 0x8E);
    adicionar_registro(8,  (unsigned)isr8,  0x08, 0x8E);
    adicionar_registro(9,  (unsigned)isr9,  0x08, 0x8E);
    adicionar_registro(10, (unsigned)isr10, 0x08, 0x8E);
    adicionar_registro(11, (unsigned)isr11, 0x08, 0x8E);
    adicionar_registro(12, (unsigned)isr12, 0x08, 0x8E);
    adicionar_registro(13, (unsigned)isr13, 0x08, 0x8E);
    adicionar_registro(14, (unsigned)isr14, 0x08, 0x8E);
    adicionar_registro(15, (unsigned)isr15, 0x08, 0x8E);
    adicionar_registro(16, (unsigned)isr16, 0x08, 0x8E);
    adicionar_registro(17, (unsigned)isr17, 0x08, 0x8E);
    adicionar_registro(18, (unsigned)isr18, 0x08, 0x8E);
    adicionar_registro(19, (unsigned)isr19, 0x08, 0x8E);
    adicionar_registro(20, (unsigned)isr20, 0x08, 0x8E);
    adicionar_registro(21, (unsigned)isr21, 0x08, 0x8E);
    adicionar_registro(22, (unsigned)isr22, 0x08, 0x8E);
    adicionar_registro(23, (unsigned)isr23, 0x08, 0x8E);
    adicionar_registro(24, (unsigned)isr24, 0x08, 0x8E);
    adicionar_registro(25, (unsigned)isr25, 0x08, 0x8E);
    adicionar_registro(26, (unsigned)isr26, 0x08, 0x8E);
    adicionar_registro(27, (unsigned)isr27, 0x08, 0x8E);
    adicionar_registro(28, (unsigned)isr28, 0x08, 0x8E);
    adicionar_registro(29, (unsigned)isr29, 0x08, 0x8E);
    adicionar_registro(30, (unsigned)isr30, 0x08, 0x8E);
    adicionar_registro(31, (unsigned)isr31, 0x08, 0x8E);
}

//Função chamada a partir da Start.asm para tratamento das interrupções
extern "C" void tratar_exception(struct regs *r)
{		
	down();
	
    if (r->int_no < 32)
    {
		//notificando exception ao escalonador
		escalonador.notificar_exception(0, r);
		
		//criando evento
		Evento evt;
		evt.num = KERNEL_PORTA_EXCEPTION;
		evt.param1 = escalonador.obter_pid_em_execucao();
		memcpy(evt.dados, (char *)r, sizeof(regs));
		
		//notificando exceção
		char msg[100];
		memcpy(msg, (char *)&evt, sizeof(Evento));
		entregador.notificar_evento(msg, KERNEL_PORTA_EXCEPTION);
	
    }
	
	up();
}

