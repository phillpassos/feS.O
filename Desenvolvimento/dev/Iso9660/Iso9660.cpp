#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Lista.h"
#include "include/Avl.h"
#include "include/Io.h"
#include "include/Ide.h"
#include "include/Iso9660.h"

Cvoldesc * disco_info;
unsigned char imagem[2048];
unsigned char setor[2048];

Lista<Cdir> diretorios_expandir;
Lista<CArquivo> diretorios_expandir_caminho;
Arvore<No> mapa;


void Iso9660_recuperar_arquivo(unsigned char * caminho, unsigned int lba, unsigned int tam)
{
   unsigned int num_set_recuperados, serv;

   int arquivo = _abrir(caminho, 'a', &serv);
   _fechar(arquivo);
  
   //calculando a quantidade de setores que precisa ser recuperada	
   unsigned int qtd_setores_recuperar = (tam + 2048 - (tam % 2048))/2048;	

   //recuperando setores e salvando no arquivo do VFS
   for(num_set_recuperados = 0; num_set_recuperados < qtd_setores_recuperar; num_set_recuperados++)	
   {
        ler_setor_atapi(caminho, lba + num_set_recuperados);
   }  
  
}

unsigned char caminho[256];
unsigned char nome[256];

void Iso9660_montar()
{
	unsigned int pos;
	CArquivo c;
	
	//obtendo informa��es do diret�rio raiz
	Cdir *cdir_raiz = (Cdir*) &disco_info->rootdir;
	
	//adicionando diret�rio raiz a lista de diret�rios a expandir
	diretorios_expandir.adicionar(*cdir_raiz);
	
	memcpy(c.val, "/cdrom", strlen("/cdrom")+1);
	diretorios_expandir_caminho.adicionar(c);
	
	//enquanto houverem diret�rios a expandir
	while(diretorios_expandir.tamanho() > 0)
	{
		//obtendo pr�ximo diret�rio (posi��o 0)
		Cdir diretorio = diretorios_expandir[0];
		
		//obtendo endere�o LBA onde o arquivo do diret�rio come�a
		unsigned int lba = *((unsigned int *)&diretorio.dloc);
		
		//obtendo tamanho do arquivo do diret�rio
		unsigned int tam_dir = *((unsigned int *)&diretorio.dlen);
		
		//recuperando arquivo do diret�rio
		Iso9660_recuperar_arquivo("/dev/disc_temp", lba, tam_dir);
			
		//alocando espa�o para armazenar informa��es sobre o diret�rio
		unsigned char * dados_diretorio = (unsigned char*) malloc(tam_dir);
		
		//reabrindo arquivo
		int temp = abrir("/dev/disc_temp", 'A');
	
		//lendo dados do diretorio
		ler(temp, dados_diretorio, tam_dir);
		fechar(temp);
			
		//calculando qtd de setores ocupada pelo diret�rio
		int qtd_setores_dir = tam_dir/2048;
		
		//varrendo todos os setores ocupados pelo diret�rio
		for(int setor_atual =0; setor_atual < qtd_setores_dir; setor_atual++)
		{
			//posi��o incial dentro de um setor para o arquivo do diret�rio 
			pos = 2048*setor_atual;
				
			//o primeiro byte armazena o tamanho do registro
			//o valor de pos � incrementado de forma a referenciar o
			//primeiro byte de cada registro. Quando o valor desse byte � zero
			//o �ltimo registro dos diret�rios para um dado setor foi alcan�ado.	
			while( ((Cdir*)&dados_diretorio[pos])->len > 0)
			{
		
				//convertendo dados em uma entrada de diret�rio
				Cdir* entrada_diretorio = (Cdir*)&dados_diretorio[pos];		
			
				//recuperando nome do arquivo/pasta
				memcpy(nome, '\0', 256);
				
				//obtendo o nome do arquivo 
				for(int aux = 0; aux < entrada_diretorio->namelen; aux++)
				{
				  if( entrada_diretorio->name[aux] != ';')
					 nome[aux] = entrada_diretorio->name[aux];		  	
				  else			  
					nome[aux] = '\0';
				}
				
				nome[entrada_diretorio->namelen] = '\0';
				
					
				//criando nome do arquivo no VFS
				
				memcpy(caminho, '\0', 256);
				memcpy(caminho, diretorios_expandir_caminho[0].val, 256);//caminho recebe caminho do pai
				strcat(caminho, "/");//adiciona barra
				strcat(caminho, nome);//adiciona nome do arquivo
						
				//adicionando apenas se n�o for indicador de diretorio pai (. ou ..)
				if(entrada_diretorio->namelen > 1)
				{
					
					//se o segundo bit for 1, essa entrada � um diret�rio
					if((entrada_diretorio->flags & 0x02))
					{
						//cria diret�rio e adiciona a lista de diret�rio a expandir
						diretorios_expandir.adicionar(*entrada_diretorio);	
						
						memcpy((unsigned char *)c.val, caminho, 256);
						diretorios_expandir_caminho.adicionar(c);
						
						//fechar(nova_pasta);
						int nova_pasta;

						adicionar_no(caminho, -1, &nova_pasta, 90);
					}
					else // arquivo comum
					{
					
				//	printf("add %s %s %d\n",nome, entrada_diretorio->name, entrada_diretorio->namelen);	
						No n; 
						int novo_descritor;
						
						//salvando localiza��o (LBA) e tamanho do arquivo
						n.lba = *((unsigned int *)&entrada_diretorio->dloc);
						n.tam = *((unsigned int *)&entrada_diretorio->dlen);
						
						//salvando nome do arquivo
						n.nome = (unsigned char*)malloc(256);
						memcpy(n.nome, caminho, 256);
					
						//cria novo n� para o arquivo				
						int ret = adicionar_no(caminho, n.tam, &novo_descritor, 90);
						
						//adicionando ao mapa de arquivos por LBA
						n.descritor = novo_descritor;
						mapa.adicionar(novo_descritor, n);
						
					}
				}
				
				//vari�vel pos sendo incrementanda com o tamanho do registro
				pos+=  (unsigned int)dados_diretorio[pos];	
			}
		}
		
		//removendo diret�rio expandido
		diretorios_expandir.remover(0);
		diretorios_expandir_caminho.remover(0);
		
		//liberando mem�ria alocada para os dados do diret�rio
		free(dados_diretorio);
	}
	
}

void Iso9660_inicializar()
{
	int tentativas = 0;
		
	//criando arquivo
	int arq = abrir("/dev/disc_info",'A');
	fechar(arq);
	
	//criando pasta em que arquivos ser�o montados
	//arq = abrir("/cdrom",'D');
	//fechar(arq);
	adicionar_no("/cdrom", -1, &arq, 90);
	
	do
	{
		//lendo setor e salvando no arquivo
		ler_setor_atapi("/dev/disc_info", 0x10);
		
		//carregando dados
		arq = abrir("/dev/disc_info",'A');	
		ler(arq, imagem, 2048);
		fechar(arq);
		
		//convertendo para ponteiro do CVoldesc
		disco_info = (Cvoldesc *)imagem;								
	
		tentativas++;
		
		if(tentativas > 10)	break;
			
	}while(disco_info->magic[0] != 0x01 || disco_info->magic[1] != 'C');
	
	//verificando se o disco foi encontrado
	if(disco_info->magic[0] == 0x01 && disco_info->magic[1] == 'C')
	{		
	  //escutar porta 90
	  escutar_porta(90);	
	  
	  //montar dados no VFS
	  Iso9660_montar();
	}
	
}

Iso9660_tratar_msg(unsigned char * msg)
{
	Bloco_Vfs * b = (Bloco_Vfs *)msg;
	int serv;

	//se o c�digo for igual a 1 (abrir arquivo)
	if(b->cod == VFS_BLOCO_ABRIR)
	{
		No& n = mapa[b->descritor];	

		if(&n != NULL)
		{
			//montar no
			montar_no(b->descritor);
			
			//recuperar arquivo e salvar no RAM DISK
			Iso9660_recuperar_arquivo(b->nome, n.lba, n.tam);
			
			
			//colocando o descritor do arquivo na mensagem 
			*(int *)msg = b->descritor;
		}		
		
		//acordad processo cliente
		enviar_msg_pid(b->pid ,msg);	

	}
	else if(b->cod == VFS_BLOCO_FECHAR)
	{
		//montar um n� j� montado, faz com 
		//que ele retorne ao estado original
		montar_no(b->descritor);
		
		//acordad processo cliente
		enviar_msg_pid(b->pid ,msg);
	}
	else if(b->cod == VFS_BLOCO_EXCLUIR)
	{
		enviar_msg_pid(b->pid ,msg);
	}
	
}

unsigned char msg[100];
main(int argc, char * args[])
{
	//inicializar driver
	Iso9660_inicializar();

	while(TRUE)
	{
		//receber mensagem
        receber_msg(msg, MSG_QLQR_PORTA);
		
		//tratar mensagem
		Iso9660_tratar_msg(msg);
	}
}