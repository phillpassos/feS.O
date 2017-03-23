#ifndef _IDT
#define _IDT

//Estrutura de uma entrada na IDT
struct registro_idt
{
    unsigned short base_low;
    unsigned short seg;             //Segmento de memória do kernel
    unsigned char  sempre_zero;     //Valor deve ser sempre 0
    unsigned char  flags;           //Valores do flag
    unsigned short base_high;
	
} __attribute__((packed));

//Ponteiro para IDT
struct idt_ptr
{
    unsigned short limite;
    unsigned int base;
	
} __attribute__((packed));

//Struct que representa o estado na pilha, quando a função de tratamento de interrupções e exceções é chamada
struct regs
{
    unsigned int gs, fs, es, ds;                           // registradores de segmento, empilhados por último
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;   // registradores empilhados pela instrução 'pusha'
    unsigned int int_no, err_code;                         // número da interrupção, e código de erro
    unsigned int eip, cs, eflags, useresp, ss;             // valores empilhados automáticamente pelo processador
	
}__attribute__((packed));

//classe IDT  
class Idt
{
	public:

	//Prótotipos	
	void adicionar_registro(unsigned char, unsigned long, unsigned short, unsigned char);
	void inicializar();
	void instalar_isrs();
};

//Definido externamente em  start.asm
extern "C" void  carregar_idt();

extern Idt idt;

#endif
