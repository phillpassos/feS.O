#include "include/Ide.h"
#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Io.h"


void ler_setor_ata(unsigned char * arq, int lba)
{
	unsigned char msg[100];
	
	//criando bloco
	Bloco_Ide b;
	b.pid = obter_pid();
	b.codigo = 0;
	b.lba = lba;
	memcpy(b.arquivo, arq, strlen(arq)+1);
	
	//convertendo para mensagem
	memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Ide));
	
	//enviando
	enviar_receber_msg(82, msg);
	
}

void escrever_setor_ata(unsigned char * arq, int lba)
{
	unsigned char msg[100];
	
	//criando bloco
	Bloco_Ide b;
	b.pid = obter_pid();
	b.codigo = 1;
	b.lba = lba;
	memcpy(b.arquivo, arq, strlen(arq)+1);
	
	//convertendo para mensagem
	memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Ide));
	
	//enviando
	enviar_receber_msg(82, msg);
}

void ler_setor_atapi(unsigned char * arq, int lba)
{
	unsigned char msg[100];
	
	//criando bloco
	Bloco_Ide b;
	b.pid = obter_pid();
	b.codigo = 2;
	b.lba = lba;
	memcpy(b.arquivo, arq, strlen(arq)+1);
	
	//convertendo para mensagem
	memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Ide));
	
	//enviando
	enviar_receber_msg(82, msg);
}

void identificar(unsigned char * arq, unsigned int bus, unsigned int drive, unsigned int tipo)
{
	unsigned char msg[100];
	
	//criando bloco
	Bloco_Ide b;
	b.pid = obter_pid();
	b.codigo = 3;
	b.param1 = bus;
	b.param2 = drive;
	b.param3 = tipo;
	memcpy(b.arquivo, arq, strlen(arq)+1);
	
	//convertendo para mensagem
	memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Ide));
	
	//enviando
	enviar_receber_msg(82, msg);
	
}

