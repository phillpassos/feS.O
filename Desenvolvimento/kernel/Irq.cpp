#include "include/Irq.h"
#include "include/Idt.h"
#include "include/Mensagem.h"
#include "include/Multiprocessamento.h"
#include "include/Util.h"


//Funções IRQ externas, definidas em start.asm
extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();
extern "C" void irq_spu();

//Instância da classe irq
Irq irq;
   
//array de ponteiros para funções, armazenam o endereço das funções de tratamento para IRQ
void * funcoes[16];

//Construtor que inicializa vetor de ponteiros
Irq::Irq(void)
{
    memset((unsigned char *) funcoes,0,sizeof(funcoes));
}

//Inicializano IRQS
void Irq::inicializar(Idt idt)
{
    remapear_irqs();
   
    idt.adicionar_registro(32, (unsigned)irq0,  0x08, 0x8E);
    idt.adicionar_registro(33, (unsigned)irq1,  0x08, 0x8E);
    idt.adicionar_registro(34, (unsigned)irq2,  0x08, 0x8E);
    idt.adicionar_registro(35, (unsigned)irq3,  0x08, 0x8E);
    idt.adicionar_registro(36, (unsigned)irq4,  0x08, 0x8E);
    idt.adicionar_registro(37, (unsigned)irq5,  0x08, 0x8E);
    idt.adicionar_registro(38, (unsigned)irq6,  0x08, 0x8E);
    idt.adicionar_registro(39, (unsigned)irq7,  0x08, 0x8E);
    idt.adicionar_registro(40, (unsigned)irq8,  0x08, 0x8E);
    idt.adicionar_registro(41, (unsigned)irq9,  0x08, 0x8E);
    idt.adicionar_registro(42, (unsigned)irq10, 0x08, 0x8E);
    idt.adicionar_registro(43, (unsigned)irq11, 0x08, 0x8E);
    idt.adicionar_registro(44, (unsigned)irq12, 0x08, 0x8E);
    idt.adicionar_registro(45, (unsigned)irq13, 0x08, 0x8E);
    idt.adicionar_registro(46, (unsigned)irq14, 0x08, 0x8E);
    idt.adicionar_registro(47, (unsigned)irq15, 0x08, 0xEE);

	for(int i = 48; i < 256; i++)
		idt.adicionar_registro(i, (unsigned)irq_spu, 0x08, 0xEE);
}
 
//Instala um nova rotina de tratamento de interrupção para o IRQ especificado
void Irq::instalar_funcao(int irq, void (*funcao)(struct regs *r))
{
    funcoes[irq] = (void *)funcao;
}

//Remove a função de tratamento para o IRQ especificado
void Irq::desinstalar_funcao(int irq)
{
    funcoes[irq] = 0;
}

void Irq::remapear_irqs()
{
	/* Geralmente, IRQs 0 até 7 são mapeados para as entradas de 8 a 15 da IDT. 
	o problema é que em modo protegido, as entradas de IDT de 0 31 são reservadas para exceptions.
	Sem o remapeamento, toda vez que IRQ0 dispara, temos uma
	Double Fault Exception, que não ocorreu de fato
	Então enviamos comandos para o (Programmable
	Interrupt Controller)  com o objetivo de remapear as IRQ0 até IRQ7 para a
	as entradas 32 a 47 da IDT */

    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
}


//função que executa o tratamendo da IRQ
extern "C" void tratar_irq(struct regs *r)
{
		down();
		
		//Criando um ponteiro para uma função em branco
		void (*funcao)(struct regs *r) = (void (*) (regs*)) funcoes[r->int_no - 32];
		
		int int_no = r->int_no;
		
		//Verifica se existe uma função de tratamento personalizada para a IRQ
		if (funcao)
		{
			funcao(r);
		}
		
		/* Enviando sinal para PIC secundário, caso número IRQ seja maior que 40 
		pois o PIC secundário trata as IRQ de 8 a 15 que estão mapeadas nas entradas
		da IDT acima de 40. Nota: O PIC secundário está ligado na IRQ2 do PIC principal
		sempre que uma IRQ entre 8 e 15 é gerada, a 2 é gerada ao mesmo tempo*/
		if (int_no >= 40)
		{
			outportb(0xA0, 0x20);
		}
			
		//Notifica o PIC do final do tratamento do IRQ
		outportb(0x20, 0x20);		
				
		//notifica o apic
		multiproc.enviar_eoi_apic();
		
		up();
}