#include "include/Sistema.h"
#include "include/Util.h"

int abrir(unsigned char * caminho, char modo)
{
	int servidor = 0, descritor;
	
	descritor = _abrir(caminho, modo, &servidor);
	
	if(servidor != 0)
	{
		Bloco_Vfs b;			 //criando bloco para o servidor do sistema de arquivos	
		b.cod = VFS_BLOCO_ABRIR; //operação de abertura de arquivo
		b.pid = obter_pid();	 //pid do processo atual para resposta
		b.descritor = descritor; //descritor do arquivo
		b.param1 = modo;
		memcpy(b.nome, caminho, strlen(caminho)+1);
		
		//copiando mensagem com o bloco
		unsigned char msg[100];
		memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Vfs));
		
		//enviando uma mensagem para o servidor do sistema de arquivos
		enviar_receber_msg(servidor, msg);
		
		//descritor = *(int *)msg;
		descritor = _abrir(caminho, modo, &servidor);
	} 

	return	descritor;
}

int ler(unsigned int descritor, unsigned char * buffer ,unsigned int tam)
{
	return _ler(descritor, buffer, tam);
}

int escrever(unsigned int descritor, unsigned char * buffer ,unsigned int tam)
{
	return _escrever(descritor, buffer, tam);
}

int buscar(unsigned int descritor, unsigned int pos)
{
	return _buscar(descritor, pos);
}

int excluir(unsigned int descritor)
{
	int ret = _excluir(descritor);
	
	if(ret == VFS_ARQUIVO_EXTERNO)
	{
		Bloco_Vfs b;			 //criando bloco para o servidor do sistema de arquivos	
		b.cod = VFS_BLOCO_EXCLUIR; //operação de exclusão de arquivo
		b.pid = obter_pid();	 //pid do processo atual para resposta
		b.descritor = descritor; //descritor do arquivo

		//copiando mensagem com o bloco
		unsigned char msg[100];
		memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Vfs));
		
		Arq_Info info;
		_obter_info_arq(descritor, (unsigned char *)&info); 
		
		//enviando uma mensagem para o servidor do sistema de arquivos
		enviar_receber_msg(info.porta_servidor, msg);
		
		//excluir
		ret = _excluir(descritor);
	} 
	
	return ret;
}

int fechar(unsigned int descritor)
{
	int ret = _fechar(descritor);
	
	if(ret == VFS_ARQUIVO_EXTERNO)
	{
		Bloco_Vfs b;			 //criando bloco para o servidor do sistema de arquivos	
		b.cod = VFS_BLOCO_FECHAR; //operação de abertura de arquivo
		b.pid = obter_pid();	 //pid do processo atual para resposta
		b.descritor = descritor; //descritor do arquivo
		
		//copiando mensagem com o bloco
		unsigned char msg[100];
		memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Vfs));
		
		Arq_Info info;
		_obter_info_arq(descritor, (unsigned char *)&info); 

		//enviando uma mensagem para o servidor do sistema de arquivos
		enviar_receber_msg(info.porta_servidor, msg);
	} 
	
	return ret;	
}

int obter_info_arq(unsigned descritor, unsigned char * dados)
{
	return _obter_info_arq(descritor, dados);
}

int adicionar_no(unsigned char * nome, unsigned int tamanho, unsigned int * novo_descritor, int porta_servidor)
{
	return _adicionar_no(nome, tamanho, novo_descritor, porta_servidor);
}

int montar_no(unsigned int novo_descritor)
{
	return _montar_no(novo_descritor);
}

int remover_no(unsigned int novo_descritor)
{
	return _remover_no(novo_descritor);
}


//Processos
int executar(unsigned char * caminho, unsigned int argc, unsigned char * args)
{
	int desc, ret, pos;
	
	//abrindo aquivo
	desc = abrir(caminho, 'b');
	
	//verificando se o arquivo foi encontrado 
	if(desc != VFS_ERR_ARQ_NAO_ENCONTRADO)
	{
		//obtendo posição final do caminho
		pos = strlen(caminho) -1;
		
		//obtendo posição do começo do nome do arquivo
		while( (pos>=0) && (caminho[pos] != '/') ) pos--;
		
		//executando arquivo
		ret = _executar(&caminho[pos+1], argc, args, desc);
		
		//fechando arquivo
		fechar(desc);
		
		//retornando ret
		return ret;
	}
	else
	{
		//retornando desc
		return desc;
	}
}

int obter_pid(void)
{
	return _obter_pid(0);
}

int obter_pid(int pid_thread)
{
	return _obter_pid(pid_thread);
}

void sistema(char * comando, Bloco_Resposta * br)
{
	Bloco_Shell bs;
	char caminho[50];
	char msg[100];
	int pid = obter_pid();
	strcat(caminho, "/dev/info");
	strcat(caminho, itoa(pid, 10));
	
	int arquivo = abrir(caminho,'a');
	escrever(arquivo, comando, strlen(comando));
	fechar(arquivo);	
	
	bs.pid = pid;
	memcpy(bs.arquivo, caminho, strlen(caminho)+1);
	
	memcpy(msg, (char *)&bs, sizeof(Bloco_Shell));
	enviar_receber_msg(1001, msg);

	memcpy((char *)br, msg, sizeof(Bloco_Resposta));
		
	//return br;
}


