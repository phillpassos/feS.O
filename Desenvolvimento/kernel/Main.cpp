#include "include/Gdt.h"
#include "include/Idt.h"
#include "include/Irq.h"
#include "include/Syscall.h"
#include "include/Memoria.h"
#include "include/Processo.h"
#include "include/Mensagem.h"
#include "include/Multiprocessamento.h"
#include "include/Loader.h"
#include "include/Util.h"
#include "include/Vfs.h"
#include "include/Multiboot.h"

void inicalizar_modulos(multiboot_info* mbd)
{
  char nome[25], nome_modulo[25];
  int pasta, arq, tam_cmd, pos, pos_param, pos_nome; 
  
  //criando diretório para armazenar módulos
  vfs.abrir("/modulos",'D', &pasta);
  vfs.fechar(pasta);
  
  //carregando módulos do grub
  mbd = (multiboot_info*)((unsigned int)mbd + POS_INICIAL_KERNEL);
  multiboot_mod_list * modulos = (unsigned int)mbd->mods_addr + POS_INICIAL_KERNEL;
 
  //varrendo todos módulos carregados no GRUB (ver arquivo menu.lst na pasta do GRUB)
  for(int i =0; i < mbd->mods_count; i++)
  {
    unsigned char * imagem = (unsigned char *) (modulos[i].mod_start + POS_INICIAL_KERNEL);
	unsigned char * cmdline = (unsigned char *) ( (int)modulos[i].cmdline + POS_INICIAL_KERNEL);
	
	//calculando tamanho do parâmetro
	tam_cmd = strlen(cmdline);
	
	//obtendo parâmetro informado no arquivo menu.lst
	for(pos = tam_cmd - 1; pos >=0; pos--)
	{
		if(cmdline[pos] == '-')
		{
			cmdline[pos] = '\0';
			pos_param = pos + 1;
			break;
		}
		else if(cmdline[pos] == ' ')
		{
			cmdline[pos] = '\0';
		}
	}
	
	//obtendo nome do módulo
	for(pos = pos_param - 2; pos >=0; pos--)
	{
		if(cmdline[pos] == '/')
		{
			pos_nome = pos + 1;
			break;
		}
		else if(cmdline[pos] == ' ')
		{
			cmdline[pos] = '\0';
		}
	}
	
	//configurando nome do arquivo
	memset(nome, '\0', 25);
	memset(nome_modulo, '\0', 25);
	memcpy(nome,"/modulos/", strlen("/modulo/")+1);
	memcpy(nome_modulo, &cmdline[pos_nome], strlen(&cmdline[pos_nome])+1);
	strcat(nome, nome_modulo);
	
	//verificando se o módulo é um driver
	if(!strcmp(&cmdline[pos_param],"DRIVER"))
	{
		//criando processo servidor
		escalonador.adicionar_processo(nome_modulo, imagem, Processo::DRIVER, 0, NULL);
	}
	//verificando se o módulo é um aplicativo que deve ser inicializado com o SO
	else if(!strcmp(&cmdline[pos_param],"APP_INI"))
	{
		escalonador.adicionar_processo(nome_modulo, imagem, Processo::USUARIO, 0, NULL);
	}
	//verificando se o módulo é o código trampolim
	else if(!strcmp(&cmdline[pos_param],"TRAMP"))
	{
		//carregando código trampolim
		multiproc.carregar_codigo_trampolim(imagem);
	}
	//verificando se o módulo é o processo IDLE
	else if(!strcmp(&cmdline[pos_param],"IDLE"))
	{
		//configurando processo IDLE
		escalonador.adicionar_idle(imagem);
	}
	
	//criando arquivo para o módulo
	vfs.abrir(nome, 'a', &arq);
	
	//escrevendo dados no arquivo
	vfs.escrever(arq, imagem, (modulos[i].mod_end - modulos[i].mod_start));
	
	//fechando arquivo
	vfs.fechar(arq);
	
  }//fim do for
  
}

//FUNÇÃO MAIN - PONTO DE PARTIDA DO SISTEMA OPERACIONAL
int main(multiboot_info* mbd, int end_fim_kernel, int bss)
{
   //inicializando controle de memória física
   mem_fisica.inicializar(mbd, end_fim_kernel);

   //inicializando controle da memória virtual
   mem_virtual.inicializar();
  
   //instalando gdt
   gdt.inicializar(mem_virtual, bss);
 
   //instalando idt
   idt.inicializar();
    
   //instalando irqs padrão
   irq.inicializar(idt);
   
   //instalando systemcalls
   syscall.inicializar(idt);
   
   //inicializando sistema de troca de mensagens
   entregador.inicializar(irq);
 
   //inicializando sistema de arquivos virtual
   vfs.inicializar();
   
   //incializando escalonador de processos
   escalonador.inicializar();
  
   //incializando módulos do GRUB
   inicalizar_modulos(mbd);

   //ativando o escalonador
   escalonador.escalonamento_ativo = 1;

   //inicializando detecção de múltiplos processadores
   multiproc.inicializar();
  
   //habilitando interrupções
   __asm__ __volatile__ ("sti");  
   
   //laço de repetição até a interrupção para carregar novo processo
   for(;;);
}
