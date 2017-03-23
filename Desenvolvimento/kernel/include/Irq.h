#ifndef _IRQ
#define _IRQ

#include "Idt.h"

class Irq
{
  public :
	
  //Protótipos	
  Irq(void);	  
  void inicializar(Idt);
  void remapear_irqs(void);
  void instalar_funcao(int , void (*handler)(struct regs *r));
  void desinstalar_funcao(int);
  
};

extern Irq irq;

#endif