#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Lista.h"
#include "include/Avl.h"
#include "include/Io.h"
#include "include/Ide.h"
#include "include/Sistimer.h"
#include "include/Fat.h"

//Declara��o de vari�veis globais
fat_BS bs;
fat_extBS_32_t ex; 
fat_info_t fat_info;
entrada_diretorio_t dir_final;

char dados[TAM_CLUSTER];
char setor_boot[TAM_CLUSTER];
char buf[TAM_CLUSTER];

Lista<entrada_diretorio> diretorio_raiz;
unsigned short info_identificar[1024];

unsigned int primeiro_setor_dados;       //N�mero do primeiro setor em que arquivos e diretorios podem ser alocados
unsigned int primeiro_setor_fat;         //Primeiro setor ocupado pela tabela FAT
unsigned int num_total_setores_dados;    //Numero total de setores de dados
unsigned int num_total_clusters;   


//---------------Comunica��o com o servidor IDE-------------------------------
void fat_ler_setor(unsigned char * dados, unsigned int setor_lba)
{
	int arq = abrir("/dev/fat_temp", 'a');
	fechar(arq);
		
	ler_setor_ata("/dev/fat_temp", setor_lba);

    arq = abrir("/dev/fat_temp", 'A');
	ler(arq,dados, TAM_CLUSTER);
	fechar(arq);
	
}

void fat_escrever_setor(unsigned char * dados, unsigned int setor_lba)
{	
	int arq = abrir("/dev/fat_temp", 'a');
	escrever(arq, dados, TAM_CLUSTER);
	fechar(arq);
	
	escrever_setor_ata("/dev/fat_temp", setor_lba);
}

void fat_identificar(unsigned char * dados, unsigned int bus, unsigned int drive, unsigned int tipo)
{	
	int arq = abrir("/dev/fat_temp", 'a');
	fechar(arq);

	identificar("/dev/fat_temp", bus, drive, tipo);
	
    arq = abrir("/dev/fat_temp", 'A');
	ler(arq, dados, TAM_CLUSTER);
	fechar(arq);
}

//---------------Manipula��o de Arquivos-------------------------------
//abre e carrega um arquivo, do disco para o VFS
void abrir_arquivo(char * nome, char * nome_vfs)
{
    //Vari�vel que armazena o primeiro cluster que o arquivo ocupa
    unsigned int cluster_inicial = 0, tamanho;
    
    //caso n�o tenha nome, o arquivo � o diretorio padr�o
    if(strlen(nome) > 0)
    {
	  
	  //varrendo diretorio raiz para encontrar cluster em que o arquivo est�
	  for(int i =0; i < diretorio_raiz.tamanho(); i++)   
	  {
	     //comparando entradas com nome
	     if(!strcmp((const char *)diretorio_raiz[i].dir_nome, nome))
		 {
			cluster_inicial = (diretorio_raiz[i].num_primeiro_clusterHI << 16) + (diretorio_raiz[i].num_primeiro_clusterLO); 
			tamanho = diretorio_raiz[i].tam_arquivo;
			break;
			
		 }// fim do if
		 
	  }//fim do for
	  
    }//fim do if
    else//se o nome for nulo, ent�o se trata do diretorio raiz
    {
      //tamanho = diretorio_raiz.tamanho() * sizeof(entrada_diretorio_t);	
      cluster_inicial = 2;
    }
  
    //Se o cluster_inicial � 0 o arquivo � foi encontrado
    if(cluster_inicial != 0)
	{
		unsigned int valor, serv;
	
		//lendo o setor em que o primeiro cluster foi armazenado
		fat_ler_setor(dados, calcular_setor_absoluto(cluster_inicial));
		
		//abrindo arquivo do VFS
		int descritor = _abrir(nome_vfs, 'a', &serv);
		
		//escrevendo dados no arquivo do VFS
		if(tamanho >= TAM_CLUSTER || (cluster_inicial == 2))
		{
			escrever(descritor, dados, TAM_CLUSTER);
			tamanho -= TAM_CLUSTER;
		}
		else
		{
			escrever(descritor, dados, tamanho);
			tamanho = 0;
		}	
		
		//printf("abrindo %s %s\n\n", nome_vfs, dados);	
		//printf("");
		
		//lendo valor contido na FAT na posi��o do primeiro cluster
		acessar_fat(cluster_inicial, LER_ENTRADA, &valor);
		
		//Repetir enquanto n�o chegar ao fim da cadeia de clusters 
		while(valor != EOC)
		{
		  //lendo os dados do setor para o pr�ximo cluster encontrado
		  fat_ler_setor(dados, calcular_setor_absoluto(valor));
		  
		  //escrevendo dados no arquivo do VFS
		  if(tamanho >= TAM_CLUSTER || (cluster_inicial == 2))
		  {
			 escrever(descritor, dados, TAM_CLUSTER);
			 tamanho -= TAM_CLUSTER;
		  }
		  else
		  {
			 escrever(descritor, dados, tamanho);
			 tamanho = 0;
		  }	
		  
		  //lendo a pr�xima entrada na FAT e salvando em : valor
		  acessar_fat(valor,LER_ENTRADA, &valor);
		  
		}
		
		//fechando arquivo do VFS
		_fechar(descritor); 
	}
}

//remove um arquivo do disco
void apagar_arquivo(char * nome)
{
	  unsigned int cluster_inicial = 0;
	  
	  //caso o nome do arquivo seja nulo, ele � o diret�rio raiz
	  if(strlen(nome) == 0)
	  {
		//cluster inicial do diret�rio raiz � 2
		cluster_inicial = 2;
	  }
	  else // sen�o, procurar arquivo pelo nome
	  {
		  // varendo as entradas do diret�rio raiz
		  for(int i =0; i < diretorio_raiz.tamanho(); i++)   
		  {
			 
			//Comparando o nome do arquivo, com as entradas do diret�rio raiz
			if(!strcmp((const char *)diretorio_raiz[i].dir_nome, nome))
			{
			   //Calculando o n� cluster inicial do arquivo. 
			   //Est� dividido em duas WORDS formando um inteiro de 32 bits. 
			   cluster_inicial = (diretorio_raiz[i].num_primeiro_clusterHI << 16) 
								  + (diretorio_raiz[i].num_primeiro_clusterLO); 

			   //removendo entrada do diretorio raiz;					  
			   diretorio_raiz.remover(i);		   
			   //salvar_diretorio_raiz();	
			   
			   break;
			}
			
		  }//fim do for
	   }// fim do else
	  
	 //Se o cluster inicial � diferente de 0, o arquivo foi encontrado
	 if(cluster_inicial != 0)
	 { 
	 
	   Lista<unsigned int> cadeia_cluster;
	   unsigned int valor;
	   unsigned int zero =0;
	  
	   //zerando o primeiro cluster depois de obter o valor dele na FAT
	   acessar_fat(cluster_inicial,LER_ENTRADA, &valor);   
	   
	   //adicionando a lista de clusters a serem zerados
	   cadeia_cluster.adicionar(cluster_inicial);
		 
	   fat_info.qtd_cluster_livres++;
		
	   //seguindo cadeia de clusters	
	   while(valor != EOC)
	   {
		  //lendo pr�xima entrada na FAT e adicionando a lista
		  cadeia_cluster.adicionar(valor);
		  //adicionando a lista de clusters a serem zerados
		  acessar_fat(valor,LER_ENTRADA, &valor);
	   }	  
		  
	   //varrenado a cadeia de clusters e zerando-os	  
	   for(unsigned int i =0; i < cadeia_cluster.tamanho(); i++)
	   {
		  //Zerando valor na entrada FAT
		  acessar_fat(cadeia_cluster[i],ALTERAR_ENTRADA, &zero);  
		  fat_info.qtd_cluster_livres++;
	   }
	   
	   //liberando mem�ria alocada pela lista de clusters a serem deletados
	   cadeia_cluster.limpar();  
		
	   //Caso o cluster inicial do arquivo seja menor que o primeiro
	   //cluster livre na fat_info, atualizar a tabela
	   if(fat_info.prox_cluster_livre > cluster_inicial)
	   {
		   fat_info.prox_cluster_livre = cluster_inicial;
	   }
	  
	 
	   //salvando FAT info pois ela foi alterada
	   salvar_fatinfo();

	  }//fim do if
  
  
 }//fim da fun��o

//carrega um arquivo do vfs para o disco
unsigned int salvar_arquivo(char * nome, char * nome_vfs)
{	
  unsigned int cluster, cluster_anterior = 0x00, cluster_ini; 
  unsigned int serv, prim_cluster = 1, ret, tamanho_total =0, tamanho =0;
  
  //apaga o arquivo caso ele j� exista no disco
  apagar_arquivo(nome);
   
  //abrindo arquivo no vfs
  int descritor = _abrir(nome_vfs,'A',&serv);
 
  //obtendo tamanho do arquivo
  Arq_Info info;
  obter_info_arq(descritor, (char *)&info);
  tamanho = info.tamanho;
 
  do //varrendo todos os clusters ocupados pelo arquivo
  {
		//caso seja o primeiro cluster, armazenar em cluster_ini
		//A fun��o retorna o primeiro cluster ocupado pelo arquivo no disco
		if(!prim_cluster)
		{
		  int prox_cluster = obter_prox_cluster_livre();
		  acessar_fat(cluster, ALTERAR_ENTRADA, &prox_cluster);   
		  cluster = prox_cluster;
		} 
		else
		{
		  prim_cluster = 0;	
		  cluster_ini = cluster = obter_prox_cluster_livre();
		}
		 
		//lendo dados do vfs 
		ret = ler(descritor, dados, TAM_CLUSTER);

		//salva os dados do cluster no disco.
		fat_escrever_setor(dados, calcular_setor_absoluto(cluster));
		
		//incrementando tamanho do aruqivo
		tamanho_total+= TAM_CLUSTER;
		
  }while(ret != VFS_EOF);//at� chegar ao fim do arquivo

  //escrevendo valor EOC no �ltimo cluster ocupado pelo arquivo	
  acessar_fat(cluster,ALTERAR_ENTRADA, &EOC);  

  //fechando arquivo do VFS 
  _fechar(descritor);   
   
  //Incluindo arquivo no diretorio ra�z
  entrada_diretorio_t en;
  memcpy(en.dir_nome,(unsigned char *)nome, strlen(nome)+1);
  
  //configurando valor do cluster inicial
  en.num_primeiro_clusterHI = (unsigned short)(cluster_ini >> 16);
  en.num_primeiro_clusterLO = (unsigned short)(cluster_ini & 0x0000FFFF);
  en.tam_arquivo			=  tamanho;
  
  //decrementando qtd de clusters livres e atualizando a fat info no disco
  fat_info.qtd_cluster_livres -= tamanho_total/TAM_CLUSTER;
  salvar_fatinfo();
  

  int tam  = strlen(nome);
  if(tam == 0)
  {
     en.dir_atributos = ATR_VOLUME_ID;
  }
  else
  {
     en.dir_atributos = ATR_ARQUIVO;
	 
	 //adicionando arquivo ao diretorio raiz
     diretorio_raiz.adicionar(en);
     //salvar_diretorio_raiz(); 
  }
 	
  //retornando valor do primeiro cluster ocupado pelo arquivo no disco
  return cluster_ini;
}


//--------------Fun��es do Servidor-------------------------
Arvore<No> arquivos;
char data[1024];
#define PORTA 91

void fat_inicializar()
{
	int arq;
	
	//escutando porta 91
	escutar_porta(PORTA);
	
	//formatar_fat32();
	carregar_fat32();
	
	//adicionando n� para a pasta do ponto de montagem
	adicionar_no("/hdd", -1, &arq, PORTA);

	//adicionar todos os arquivos do disco
	for(int i =0; i < diretorio_raiz.tamanho(); i++)
	{
		//obtendo entrada
	    entrada_diretorio& arquivo = diretorio_raiz[i];
		
		//montando caminho do arquivo
		char caminho[100] = "/hdd/";
		strcat(caminho, arquivo.dir_nome);
		
		//adicionando n�
		adicionar_no(caminho, arquivo.tam_arquivo, &arq, PORTA);
		
		//adicionando a lista de n�s
		No no;
		no.descritor = arq;
		no.arq = &arquivo;
		no.nome = (char * )malloc(sizeof(char)*80);
		memcpy(no.nome, caminho , 80);
		arquivos.adicionar(no.descritor, no);
		
	}
	
}


No no;
void fat_tratar_solicitacoes(char * msg)
{
	//convertendo mensagem em bloco
	Bloco_Vfs * b = (Bloco_Vfs *)msg;
	int serv, desc;

	//se o c�digo for igual a 1 (abrir arquivo)
	if(b->cod == VFS_BLOCO_ABRIR)
	{
		//obtendo dados do n�
		No& n = arquivos[b->descritor];	

		//caso dados n�o sejam encontrados, criar novo arquivo
		if(&n != NULL)
		{
			//montar n� vfs
			montar_no(b->descritor);
			
			//carregar arquivo
			abrir_arquivo(n.arq->dir_nome, b->nome);

			//colocando o descritor do arquivo na mensagem 
			*(int *)msg = b->descritor;
		}
		else		
		{
			//para impedir a crian��o de diret�rios
			if(b->param1 != 'D')
			{
				//adicionando novo n�
				adicionar_no(b->nome, 0, &desc, PORTA);
				
				//montando n� para permitir acesso ao arquivo
				montar_no(desc);
				
				//adicionando n� a lista de n�s
				no.descritor = desc;
				no.nome = (char * )malloc(sizeof(char)*80);
				memcpy(no.nome, b->nome ,strlen(b->nome)+1);
				arquivos.adicionar(no.descritor, no);			
			}
				
		}
		
		//acordando processo cliente
		enviar_msg_pid(b->pid ,msg);	
  
	}
	else if(b->cod == VFS_BLOCO_FECHAR)
	{
		//recuperar do RAMDISK e salvar no disco
		No& n = arquivos[b->descritor];	
		
		
		//teste para verificar se o n� foi encontrado
		if(&n != NULL)
		{
			//obtendo posi��o do come�o do nome do arquivo	
			int pos = strlen(n.nome) -1;
			while(n.nome[pos] != '/') pos--;
			pos++;
			
			//salvando arquivo no disco
			salvar_arquivo(&n.nome[pos], n.nome);
			
			//atualizando diret�rio raiz
			salvar_diretorio_raiz();
			
			//montar o bloco j� montado
			//resutara ele ao estado anterior
			montar_no(b->descritor);
		}	
			
	    //acordando processo cliente		
		enviar_msg_pid(b->pid ,msg);
	}
	else if(b->cod == VFS_BLOCO_EXCLUIR)
	{
		//remover arquivo do disco
		No& n = arquivos[b->descritor];	
		
		//teste para verificar se o n� foi encontrado
		if(&n != NULL)
		{
			//obtendo posi��o do come�o do nome do arquivo	
			int pos = strlen(n.nome) -1;
			while(n.nome[pos] != '/') pos--;
			pos++;
			
			//salvando arquivo no disco
			apagar_arquivo(&n.nome[pos]);
			
			//removendo no do VFS
			remover_no(b->descritor);
			
			//atualizando diret�rio raiz
			salvar_diretorio_raiz();
		
			enviar_msg_pid(b->pid ,msg);
		}
	}
	
}

//declara��o da mensagem
unsigned char msg[100];

main(int argc, char * args[])
{
	//inicializar driver
	fat_inicializar();
 
	while(TRUE)
	{
		//receber mensagem
        receber_msg(msg, MSG_QLQR_PORTA);
			
		//tratar mensagem
		fat_tratar_solicitacoes(msg);
	}
}