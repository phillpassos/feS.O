#ifndef _MULTIPROC
#define _MULTIPROC

#include "Memoria.h"
#include "Loader.h"
#include "Avl.h"

struct MPFloatPoint
{
	unsigned char assinatura[4]; 	//assinatura "_MP_"
	unsigned char * config_ptr; 	//Poteiro para a MP Configuration Table
	unsigned char tam; 				//tamanho da estrutura
	unsigned char versao;			//vers�o da especifica��o
	unsigned char checksum;			
	unsigned char mp_feat1; 	    //flags de caracter�sticas
	unsigned char mp_feat2; 	    //flags de caracter�sticas
	unsigned char mp_feat3_5[3]; 	//flags de caracter�sticas
	
}__attribute__((packed));

struct MPConfigTable
{
	unsigned char assinatura[4];	//assinatura "PCMP"
	unsigned short tam;				//tamanho da tabela base
	unsigned char spec_rev;			//indica a vers�o
	unsigned char checksum;			
	unsigned char oemid[8];			//string com id do fabricante	 	
	unsigned char productid[12];    //string com id do produto
	unsigned int * oem_table_ptr;	//ponteiro para oem table
	unsigned short oem_table_tam;	//tamanho da oem table
	unsigned short qtd_itens;		//Quantidade de itens na MP Config Table
	unsigned int lapic_add;			//Endere�o f�sico do local do LAPIC
	unsigned short xtable_tam;		//Tamanho da Extended Table
	unsigned short xtable_checksum;
	
}__attribute__((packed));

struct MPConfigTableProc
{
	unsigned char tipo;			 	 //entrada da MPConfig Table referente a um processador (valor 0)
	unsigned char lapicid;  		 //Id do Local APIC
	unsigned char lapicver; 	 	 //vers�o do LAPIC
	unsigned char cpu_enable_bsp;	 //Bit 0 indica se a CPU est� habilitada, e o 1 se � o Bootstrap processor
	unsigned char cpu_assinatura[4]; //Assinatura da CPU
	unsigned char cpu_flags[4];		 //Flags da CPU	
	
}__attribute__((packed));

struct MPConfigTableIOApic
{
	unsigned char tipo;				//entrada da MPConfig Table referente a um processador (valor 0)
	unsigned char iopicid;  		//Id do IOAPIC
	unsigned char ioapicver; 		//vers�o do IOAPIC
	unsigned char iopaic_enable;	//Bit 7 indica se IO apic est� habilitado
	unsigned int ioapic_add;		//Endere�o f�sico da localiza��o do IO APIC

}__attribute__((packed));


struct MPConfigInterrupt
{
	unsigned char tipo;				//entrada da MPConfig Table referente a uma interrupt (valor 3)
	unsigned char tipo_interrupcao; //tipo da interrupcao
	unsigned char flag[2]; 			//flags
	unsigned char source_bus;		//barramento de origem
	unsigned char source_bus_irq;	//
	unsigned char idioapicdest;		//id do ioapic de destino
	unsigned char indice;			//id do ioapic de destino

}__attribute__((packed));


struct RSDP 
{
	unsigned char assinatura[8];
	unsigned char checksum;
	unsigned char oemid[6];
	unsigned char revisao;
	unsigned int rsdt_end;
  
}__attribute__((packed));

struct RSDT_Header 
{
  unsigned char assinatura[4];
  unsigned int tam;
  unsigned char revisao;
  unsigned char checksum;
  unsigned char oemid[6];
  unsigned char oemtableid[8];
  unsigned int oemrevisao;
  unsigned int criadorid;
  unsigned int criadorrevisao;
  
}__attribute__((packed));

struct RSDT
{
	RSDT_Header cabecalho;
	unsigned int conteudo[200];
	
}__attribute__((packed));

struct MADT_entrada
{
	unsigned char tipo;
	unsigned char tam;
	
}__attribute__((packed));

struct MADT_Header
{
	RSDT_Header cabecalho;
	unsigned int lapic_end;
	unsigned int flags;

}__attribute__((packed));

struct MADT
{
	MADT_Header cabecalho;
	unsigned int * conteudo;
	
}__attribute__((packed));


struct ACPI_Lapic
{
	MADT_entrada cabecalho;
	unsigned char idprocessador;  //id do processador
	unsigned char apicid;		  //id do apic
	unsigned int flags;			  //bit 0 indica se o processador est� habilitado 
								  //(outros bits est�o reservados)
}__attribute__((packed));

struct ACPI_Ioapic
{
	MADT_entrada cabecalho;
	unsigned char ioapicid;		  //id do i/o apic
	unsigned char reservado;	  //reservado
	unsigned int ioapic_end;	  //endere�o do i/o apic
	unsigned int gsib;			  //n�mero da primeira interrup��o do i/o apic
	
}__attribute__((packed));


struct ACPI_Interrupt_Source
{
	MADT_entrada cabecalho;
	unsigned char bus;				//Bus = 0 = ISA
	unsigned char origem;			//N�mero da IRQ do bus
	unsigned int gsi;				//N�mero da interrup��o para o i/o apic
	unsigned char flags[2];			//Flags, bits (0, 1) = polaridade, bits(2, 3) = trigger mode
	
}__attribute__((packed));


class Multiproc
{
	Lista<MPConfigTableIOApic> ioapics;
	Lista<MPConfigTableProc> processadores;
	Lista<MPConfigInterrupt> interrupcoes;
	Arvore<int> apics;

	int multiproc_habilitado;
	unsigned int apic;
	unsigned int ioapic;
	unsigned int entrada;
	
	public:
		
	volatile int processadores_iniciados;	
	
	void inicializar();
	void inicializar_ioapic();
	void habilitar_apic();
	void habilitar_apic_timer();
	void inicializar_ioapic2();
	void analisar_madt(unsigned int * end);
	void carregar_codigo_trampolim(char * imagem);
	void analisar_MPConfigTable(char * ptr);
	void escrever_reg_apic(int reg, int dado);
	void escrever_reg_ioapic(int reg, int dado);
	void enviar_eoi_apic();
	int buscar_floating_point_struc(MPFloatPoint * mpfloatpoint);
	int buscar_rsdt();
	int obter_apicid();
	unsigned int ler_reg_apic(int reg);
	unsigned int ler_reg_ioapic(int reg);

};


extern volatile Multiproc multiproc;
extern "C" volatile int test_and_set(int novo_valor, int * ptr);
void trampolim();
void down();
void up();

#endif