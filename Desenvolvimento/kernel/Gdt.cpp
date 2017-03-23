#include "include/Gdt.h"
#include "include/Util.h"

//Ver arquivo start.asm	para definição da estrutura
struct gdt_ptr gp;

//Instância da global da classe GDT
volatile Gdt gdt;

//Função resposável por configurar a GDT
void Gdt::inicializar(Memoria_virtual mem_virtual, int bss)
{
    //Configurando o ponteiro para o GDT 
    gp.limite = (sizeof(struct registro_gdt) * 24) - 1; //O limite da GDT para 24 registros (8registros básicos + 1 TSS por CPU (max 16))
    gp.base   = (unsigned int) &this->registros;	    //O Endereço base da GDT é o endereço do array
	
    //Adicionando registro nulo. (O Primeiro registro da GDT dever ser nulo)
    adicionar_registro(0, 0, 0, 0, 0);

    //Adicionando entradas na GDT. A base é Sempre 0, o Limite é de 4GB (flat memory model)
    adicionar_registro(1, 0, 0xFFFFFFFF, Gdt::ANEL0_CODIGO, 0xCF); //indice 0x08
    adicionar_registro(2, 0, 0xFFFFFFFF, Gdt::ANEL0_DATA,   0xCF); //indice 0x10
    adicionar_registro(3, 0, 0xFFFFFFFF, Gdt::ANEL1_CODIGO, 0xCF); //indice 0x18
    adicionar_registro(4, 0, 0xFFFFFFFF, Gdt::ANEL1_DATA,   0xCF); //indice 0x20
    adicionar_registro(5, 0, 0xFFFFFFFF, Gdt::ANEL3_CODIGO, 0xCF); //indice 0x28
    adicionar_registro(6, 0, 0xFFFFFFFF, Gdt::ANEL3_DATA,   0xCF); //indice 0x30

	//instalando o TSS
	unsigned int end = (unsigned int)&tss;
	
    //adicionar_registro(7, end, end+sizeof(Tss), 0xE9, 0x00);	
	adicionar_registro(7, end, sizeof(Tss) -1, 0xE9, 0x00);	
	
	//limpando estrutura da TSS
	memset((unsigned char *)&tss, 0, sizeof(tss));
	
	//configurando os campos da TSS
	tss.iomap = (unsigned short) sizeof(Tss);
	tss.esp0  = bss;
	tss.ss0   = 0x10;
	tss.cs    = 0x00;
    tss.ss    = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

	//Apagando a GDT antiga (falsa) e instalando a nova
	carregar_gdt();

	//Alterando o registrador que armazena a posição do TSS, 0x38 representa a 7ª entrada na GDT
	asm volatile(" mov $0x38, %ax; ltr %ax"); //asm volatile(" mov $0x2B, %ax; ltr %ax");
	
	//configurando a quantidade de registros presente na GDT
	qtd_registros = 8;
	
	//removendo mapeamento da primeira page table após a GDT original ser instaladas		 
	mem_virtual.remover_entrada_inicial();
}

//Adicionar um novo registro à Global Descriptor Table 
void Gdt::adicionar_registro(int num, unsigned long base, unsigned long limite, unsigned char accesso, unsigned char gran)
{
    //Configura o endereço base dos registro
    this->registros[num].base_low     = (base & 0xFFFF);
    this->registros[num].base_meio    = (base >> 16) & 0xFF;
    this->registros[num].base_high    = (base >> 24) & 0xFF;

    //Configura o limite do registro 
    this->registros[num].limite_low    = (limite & 0xFFFF);
    this->registros[num].granularidade = ((limite >> 16) & 0x0F);

    //Configura a granularidade e o nível de acesso (Ring0, Ring1, Ring3)
    this->registros[num].granularidade |= (gran & 0xF0);
    this->registros[num].accesso       = accesso;
}

//adiciona um novo registro do tipo TSS à Global Descritor Table
void Gdt::adicionar_tss(unsigned int pilha)
{
	//alocando memória para o TSS
	Tss * nova_tss	= (Tss*)kmalloc(sizeof(Tss));
	
	//limpando estrutura da TSS
	memset((unsigned char *)nova_tss, 0, sizeof(Tss));
	
	//adicionando um novo registro do tipo tss na GDT;	
	adicionar_registro(qtd_registros, nova_tss, sizeof(Tss) -1, 0xE9, 0x00);	
	
	//configurando os campos da TSS
	nova_tss->iomap = (unsigned short) sizeof(Tss);
	nova_tss->esp0  = pilha;
	nova_tss->ss0   = 0x10;
	nova_tss->cs    = 0x00;
    nova_tss->ss    = nova_tss->ds = nova_tss->es = nova_tss->fs = nova_tss->gs = 0x13;
	
	//incrementando a quantidade de registros na GDT
	qtd_registros++;
}

//Altera o endereço para o registrador ESP que será carregado quando uma interrupção ocorrer
void Gdt::alterar_tss_esp(unsigned int esp)
{
	tss.esp0 = esp;
}