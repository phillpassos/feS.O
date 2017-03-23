#include "include/Util.h"
#include "include/Io.h"
#include "include/Sistema.h"


// processo de tempo ocioso do sistema(idle)
main(int argc, char * args[])
{	
	//ocupando CPU
	for(;;)
		 asm volatile("hlt"); //paralizando CPU até que uma interrupção ocorra
    //sair();
}