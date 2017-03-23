#ifndef _IDE_USR
#define _IDE_USR

struct Bloco_Ide
{
   int codigo;
   int pid;
   int lba;
   int param1;
   int param2;
   int param3;
   unsigned char arquivo[50]; 
};

void ler_setor_ata(unsigned char * arq, int lba);
void escrever_setor_ata(unsigned char * arq, int lba);
void ler_setor_atapi(unsigned char * arq, int lba);
void identificar(unsigned char * arq, unsigned int bus, unsigned int drive, unsigned int tipo);
#endif