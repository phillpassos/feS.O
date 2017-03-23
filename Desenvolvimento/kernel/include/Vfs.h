#ifndef _VFS
#define _VFS

#include "Avl.h"
#include "Lista.h"


//constantes do sistem de arquivos
const int VFS_SUCESSO = 1;
const int VFS_ARQUIVO_EXTERNO = -2;
const int VFS_EOF = -2;
const int VFS_POSICAO_INVALIDA = -5;
const int VFS_ARQUIVO_FECHADO = -6;
const int VFS_ARQUIVO_ABERTO = -7;
const int VFS_DIR_NAO_ENCONTRADO = -8;
const int VFS_SEM_PERMISSAO = -9;
const int VFS_ERR_TAM_MAX_DIR = -10;
const int VFS_ERR_ARQ_NAO_ENCONTRADO = -11;
const int VFS_NOME_INVALIDO = -12;
const int VFS_TENTATIVA_ESCREVER_PASTA = -13;


#define EOF -1
#define MAX_TAM_ARQ 15

struct Bloco
{
	unsigned char dados[512];
};

class Diretorio
{
	static int const MAX_ARQ = 127;
	
	public:
	
	int qtd_arquivos;
	unsigned int arqs[127];
    int adicionar_arquivo(unsigned int);
	int remover_arquivo(unsigned int);
	
} __attribute__((packed));

class Arquivo : public Recurso
{
	public :
	
	unsigned char nome[MAX_TAM_ARQ];
	unsigned int offset;
	unsigned int tamanho;
	unsigned int tamanho_em_disco;
	unsigned int pai;
	unsigned int tipo;
	unsigned int descritor;
	unsigned int porta_servidor;
	unsigned int flags;
	int aberto_por;
	Lista<Bloco> clusters;
	
	Arquivo()
	{
		id_dono          =-1;
		mutex 	         =0;
		tamanho 		 =0;
		tamanho_em_disco =0;
	}
};

struct Arq_Info
{
	unsigned char nome[MAX_TAM_ARQ];
	unsigned int offset;
	unsigned int tamanho;
	unsigned int tamanho_em_disco;
	unsigned int pai;
	unsigned int tipo;
	unsigned int descritor;
	unsigned int porta_servidor;
	unsigned int flags;
	int aberto_por;
	char enchimento[60];
}__attribute__((packed));

struct Bloco_Vfs
{
	unsigned int cod;
	unsigned int pid;
	unsigned int descritor;
	
}__attribute__((packed));

class Vfs : public Recurso
{
	static const int ARQ = 0;
	static const int DIR = 1;
	static const int MNT = 2;
	static const int MNT_DIR = 3;
	static const int MNT_ABERTO = 4;

	Arquivo raiz;
	Arvore<Arquivo> arquivos;
	unsigned int prox_descritor;
	Bloco novo_bloco;
	
	unsigned int criar_arquivo(Arquivo,unsigned char *, char, unsigned int *);
	unsigned int _abrir(unsigned char *, char, unsigned int *, unsigned int *);
	unsigned int validar_nome(unsigned char *);
	
	public :
	
	void inicializar();
	Arquivo& obter_arquivo(unsigned int);
	unsigned char * obter_imagem(unsigned int);
	unsigned int abrir(unsigned char *, char, unsigned int *);
	unsigned int abrir(unsigned char *, char, unsigned int *, unsigned int *);
	unsigned int escrever(unsigned int, unsigned char *, unsigned int);
	unsigned int ler(unsigned int, unsigned char *, unsigned int);
	unsigned int buscar(unsigned int, unsigned int);
	unsigned int excluir(unsigned int);
	unsigned int fechar(unsigned int);
	unsigned int adicionar_no(unsigned char *, int, unsigned int, unsigned int *);
	unsigned int montar_no(unsigned int);
	unsigned int remover_no(unsigned int);
	unsigned int obter_info_arquivo(unsigned, unsigned char *);

};


extern volatile Vfs vfs;





#endif
