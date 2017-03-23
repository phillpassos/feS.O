#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Avl.h"
#include "include/Lista.h"
#include "include/Teclado.h"
#include "include/Video.h"

//estrutura para comunica��o
struct Bloco_Console
{
	unsigned int codigo;
	unsigned int pid;
	unsigned int param1, param2;
	unsigned int tam;
	unsigned char in[35];
	unsigned char out[35];
	
}__attribute__((packed));

//estrutura para armazenar informa��es sobre clientes
struct Cliente
{
	unsigned int pid;
	unsigned char in[50];
	unsigned char out[50];
	unsigned char ativo;
	unsigned char atualizado;
	Vid_Status * status;
	Tec_Status * tec_status;
};

//declara��o da estrutura que armazena informa��es sobre os clientes
Arvore<Cliente> clientes;
Lista<int> fila_clientes;
int pid_foreground = 0;

//lista de solicita��es
Lista<Bloco_Console> solicitacoes;

void alterar_processo_foreground()
{
	if(fila_clientes.tamanho() > 0)
	{
		//obtendo novo cliente e removendo da fila
		int prox_pid = fila_clientes[0];
		fila_clientes.remover(0);
		
		//obtendo informa��es sobre o prox cli e o atual 
		Cliente &prox_cli = clientes[prox_pid];
		
		//salvando dados processo atual
		if(pid_foreground != 0)
		{
		    //adicionando cliente atual ao fim da fila
		    fila_clientes.adicionar(pid_foreground);
			
			//salvando processo atual
			Cliente &cli = clientes[pid_foreground];
			vid_recuperar_status(cli.status);
			tec_recuperar_status(cli.tec_status);
		}
		
		//colocando pr�ximo processo
		tec_alterar_processo_foreground(prox_cli.tec_status);
		vid_alterar_processo_foreground(prox_cli.status);
		
		
		//alterando para o pid em foreground
		pid_foreground  = prox_pid;
		
		
	}
}

void adicionar_solicitacao(char * msg, int id_msg)
{
	Bloco_Console * dados = (Bloco_Console *)msg;
	
	/*
	  verificando se o pid no bloco de dados � igual ao emissor
	  da mensagem retornando pela sistema operacional.
	*/   
	if(dados->pid != id_msg)
	{
		//Caso n�o seja, verificar se � o caso de duas threads de um mesmo processo
		dados->pid =  obter_pid(id_msg) == dados->pid ? dados->pid : id_msg;
		
	}

	Cliente &c = clientes[dados->pid];
	
	//caso o cliente ainda n�o exista, criar
	if(&c == NULL)
	{
		Cliente novo_cli;
	
		//novo_cli.pid = dados->pid;
		novo_cli.pid = dados->pid;
		novo_cli.atualizado = 1;
		novo_cli.ativo = 1;
		
		//configurando dados do video	   
		novo_cli.status = (Vid_Status *)malloc(sizeof(Vid_Status));
		novo_cli.status->vid_pid_foreground = id_msg;
		
		novo_cli.status->atributo = VID_COLOR_BRANCO;
		novo_cli.status->cursor_habilitado = 0;
		novo_cli.status->mover_cursor_auto = 1;
		novo_cli.status->csr_x =0;
		novo_cli.status->csr_y =1;
		
		//criando nome do arquivo
		memcpy(novo_cli.status->terminal_backup,"/dev/tty", strlen("/dev/tty")+1);		
		strcat(novo_cli.status->terminal_backup, itoa(dados->pid,10));
				
		//declarando linha em branco 
		short linha[80], branco = 0x20 | (VID_COLOR_BRANCO << 8);
		memsetw(linha, branco, 80);
		
		//abrindo novo arquivo
		int arq = abrir(novo_cli.status->terminal_backup, 'a');
		
		//escrevendo linhas em branco
		for(int i =0; i < 25;i++) escrever(arq, (unsigned char *)linha, 80*2);
		
		//fechando o arquivo
		fechar(arq);
			
		//configurando dados do teclado
		novo_cli.tec_status = (Tec_Status *)malloc(sizeof(Tec_Status));
		memset((char *)novo_cli.tec_status, 0, sizeof(Tec_Status));
		novo_cli.tec_status->teclado_ativado     = 0;
		novo_cli.tec_status->ecoamento           = 1;
		novo_cli.tec_status->modo_canonico       = 0;
		novo_cli.tec_status->tec_pid_foreground  = id_msg;
		
		//atualizando arquivos do cliente		
		memcpy(novo_cli.in, dados->in, strlen(dados->in)+1);
		memcpy(novo_cli.out, dados->out, strlen(dados->out)+1);   
		memcpy(novo_cli.status->vid_arquivo_out, dados->out, strlen(dados->out)+1);   
		memcpy(novo_cli.tec_status->arquivo, dados->in, strlen(dados->in)+1);
	
		//adicionando a lista de clientes
		clientes.adicionar(novo_cli.pid, novo_cli);		
		fila_clientes.adicionar(novo_cli.pid);	
	}
	else if (c.ativo)
	{
	
		if( dados->codigo == LER_TECLAS || dados->codigo == ALTERAR_CONF_TECLADO)
		{
			c.tec_status->tec_pid_foreground = id_msg;
		}
		else
		{
			c.status->vid_pid_foreground = id_msg;
		}
		
		//atualizando arquivos do cliente
		memcpy(c.out, dados->out, strlen(dados->out)+1);
		memcpy(c.in, dados->in, strlen(dados->in)+1);   
	}
	
	//caso nenhum processo esteja em primeiro plano novo processo em primeiro plano
	if(pid_foreground == 0)
	{
		alterar_processo_foreground();
	}
	
	
	//adicionando nova solicita��o a lista
	solicitacoes.adicionar(*dados);
}

void remover_cliente(char * msg)
{
    //evendo sinalizado pelo SO
	Evento * evt = (Evento*)msg;
	
	//verificando se o evento corresponde a um processo sendo destru�do
	if(evt->num == KERNEL_PORTA_PROC_DESTRUIDO)
	{
		//obter pid do cliente
		int pid_cliente = evt->param1;
		
		//obter dados do cliente
		Cliente &cli = clientes[pid_cliente];
			
		if(&cli != NULL)
		{
			//caso o cliente esteja em primeiro plano
			if(pid_cliente == pid_foreground)
			{
				//colocar outro cliente em primeiro plano
				alterar_processo_foreground();
			}
			
			//excluindo backup do terminal
			int arq = abrir(cli.status->terminal_backup, 'a');
			excluir(arq);
			
			//excluindo arquivo de sa�da
			arq = abrir(cli.status->vid_arquivo_out, 'a');
			excluir(arq);
			
			//excluindo arquivo de entrada
			arq = abrir(cli.tec_status->arquivo, 'a');
			excluir(arq);
		
			//liberando mem�ria ocupada pelos dados do cliente
			free(cli.status);
			free(cli.tec_status);
			
			//desativando cliente
			///clientes.remover(pid_cliente);
			cli.ativo = 0;
			
			//varrendo fila de clientes, para remover o cliente
			for(int i =0; i < fila_clientes.tamanho(); i++)
			{
				if(fila_clientes[i] == pid_cliente)
				{
					fila_clientes.remover(i);
					break;
				}
			}
		}
	}
}

void tratar_solicitacao()
{
	int solicitacao_executavel = 1, i;
	
	//enquanto houver solicita��es a atender
	while(solicitacao_executavel)
	{
		Bloco_Console b;
		solicitacao_executavel = 0;
		
		//varrendo todas as solicita��es
		for(i = 0; i < solicitacoes.tamanho(); i++)
		{
			b = solicitacoes[i];
			if(b.pid == pid_foreground 
			|| ( b.codigo != LER_TECLAS &&  b.codigo != ALTERAR_CONF_TECLADO && b.codigo !=  HABILITAR_CURSOR ))
			{
				//solicitaca��o do processo em foreground encontrada
				solicitacao_executavel = 1;
				break;
			}
		}
		
		//caso uma solicita��o execut�vel tenha sido encontrada
		if(solicitacao_executavel)
		{
			
			Vid_Status status;
			Cliente& cliente = clientes[b.pid];
			
			if(b.pid != pid_foreground)
			{
				vid_mudar_para_background(&status, cliente.status);
			}

			//decidindo tipo de solicitacao
			switch(b.codigo)
			{
				case IMPRIMIR       	  : vid_atender_solicitacao(b.codigo, b.out, b.param1, b.param2, b.tam); break;
				case ALTERAR_POS 		  : vid_atender_solicitacao(b.codigo, b.out, b.param1, b.param2, b.tam); break;
				case LER_TECLAS  		  : tec_atender_solicitacao(cliente.tec_status->tec_pid_foreground, clientes[b.pid].in); break;
				case ALTERAR_CONF_TECLADO : tec_alterar_estado(cliente.tec_status->tec_pid_foreground, b.param1, b.param2);break;
				case CONFIG_COR  		  : vid_atender_solicitacao(b.codigo, b.out, b.param1, b.param2, b.tam); break;
				case HABILITAR_CURSOR 	  : vid_atender_solicitacao(b.codigo, b.out, b.param1, b.param2, b.tam); break;
				case LIMPAR_TELA 		  : vid_atender_solicitacao(b.codigo, b.out, b.param1, b.param2, b.tam); break;
				case LER_VIDEO_MEM		  : vid_atender_solicitacao(b.codigo, b.out, b.param1, b.param2, b.tam); break;	
				case ESC_VIDEO_MEM		  : vid_atender_solicitacao(b.codigo, b.out, b.param1, b.param2, b.tam); break;
			}
			
			if(b.pid != pid_foreground)
			{
				vid_mudar_para_foreground(&status, cliente.status);
			}
				
			//removendo solicita��o da lista
			solicitacoes.remover(i);
			
		}//fim do if
		
	}//fim do while
}

//fun��o principal do driver
main(int argc, char * args[])
{
	//declarando mensagem
	unsigned char mensagem[100];
	
	//escutar porta 1 (IRQ do teclado)
	escutar_porta(1);
	
	//escutar evento do kernel
	escutar_porta(KERNEL_PORTA_PROC_DESTRUIDO);
	
	//escutar porta 80 comunica��o com clientes
	escutar_porta(80);
			
	//inicializando v�deo
    vid_inicializar();
  
	//incializar teclado
    tec_inicializar(); 	

		
	//la�o de repeti��o infinito do servidor
	while(TRUE)
	{
		//aguarando mensagem do cliente e/ou IRQ
		int tipo = receber_msg(mensagem, MSG_QLQR_PORTA);
	
		//caso o conte�do da mensagem seja IRQ
		if(tipo == MSG_IRQ)
		{
			//tratando pressionamento
			int alterar_foreground = tec_tratar_pressionamento();
			
			//verificando solicita��o de altera��o de processo
			if(alterar_foreground)
			{
				alterar_processo_foreground();
			}
		}
		else if(tipo == MSG_EVT)
		{
			remover_cliente(mensagem);
		}
		else
		{
			adicionar_solicitacao(mensagem, tipo);
		}
		
		//tratar solicita��es pendentes caso existam
		tratar_solicitacao();
		
	}

}