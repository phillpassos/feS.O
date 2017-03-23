#include "include/Editor.h"
#include "include/Io.h"
#include "include/Sistema.h"

//--------------------------------------------------------------------------------------------------------
/* DEFINI��ES DA CLASSE Texto  */
void Texto::adicionar_linha()
{
    //adicionando nova linha a lista de linhas
    Lista<unsigned char> nova_linha;
	conteudo.adicionar(nova_linha);
}

void Texto::remover_char(unsigned int pos_x, unsigned int pos_y)
{
	conteudo[pos_y].remover(pos_x);
}

void Texto::adicionar_char(unsigned char c, unsigned int pos_x, unsigned int pos_y)
{	
	conteudo[pos_y].adicionar_em(pos_x, c);
}

void Texto::dividir_linha(unsigned int pos_x, unsigned int pos_y, unsigned int tam_max)
{
	Lista<unsigned char> nova_linha;
	
	//obtendo a linha
	//Lista<unsigned char> *linha;
	Lista<unsigned char>& linha = conteudo[pos_y];
	//conteudo.remover(pos_y);
	
	//removendo os caracteres da linha e adicionando � linha nova
	unsigned int tam_ini = linha.tamanho();
	for(unsigned int i = pos_x; i < tam_ini; i++)
	{
		unsigned char aux = linha[pos_x];
		linha.remover(pos_x);
		nova_linha.adicionar(aux);
	}
		
	//readicionando a linha, e adicionando a nova linha
	//conteudo.adicionar_em(pos_y, *linha);
	conteudo.adicionar_em(pos_y+1, nova_linha);
}

void Texto::unir_linhas(unsigned int pos_linha, unsigned int tam_max)
{
	//obtendo linha superior
	Lista<unsigned char> *linha_superior;
	linha_superior = &conteudo[pos_linha-1];
	
	//obtendo linha
	Lista<unsigned char> *linha;
	linha = &conteudo[pos_linha];
	
	//caso a linha superior j� possua o tamanho m�ximo, remover �ltimo caractere
	if(linha_superior->tamanho() == tam_max)
	{
		linha_superior->remover(linha_superior->tamanho() - 1);
	}
	
	//adiciona cacacteres da linha inferior at� que ela fique vazia
	//ou at� q a linha superior fique cheia
	while( (linha_superior->tamanho() < tam_max ) && (linha->tamanho() > 0) )
	{
		linha_superior->adicionar((*linha)[0]);
		linha->remover(0);	
	}

	//salvando altera��es na lista
	conteudo.remover(pos_linha - 1);
	conteudo.adicionar_em(pos_linha-1, *linha_superior);
	
	
	conteudo.remover(pos_linha);
	
	//caso n�o tenham sobrado caracteres na lista, ela � removida e n�o mais adicionada
	if(linha->tamanho() > 0)
		conteudo.adicionar_em(pos_linha, *linha);
}

Lista<unsigned char>& Texto::operator[](unsigned long indice)
{
   return conteudo[indice];
}

int Texto::tamanho()
{
   return conteudo.tamanho();
}

//--------------------------------------------------------------------------------------------------------
/* DEFINI��ES DA CLASSE AreaDeTexto  */
short AreaDeTexto::obter_x_abs(short x_rel)
{
  return x_rel + x_ini + 1;
}

short AreaDeTexto::obter_y_abs(short y_rel)
{
  return y_rel + y_ini + 1;
}

AreaDeTexto::AreaDeTexto(){}

AreaDeTexto::AreaDeTexto(short x_i, short y_i, short x_f, short y_f)
{
  x_ini = x_i;
  x_fim = x_f;
  y_ini = y_i;
  y_fim = y_f;
  
  colunas_max = x_fim - x_ini - 1;
  linhas_max  = y_fim - y_ini - 1;
     
  pos_x = 0;
  pos_y_texto = 0;
  pos_y_tela  = 0;
  prim_linha_exibida = 0; 
   
  texto.adicionar_linha();
}

void AreaDeTexto::inicializar(char * titulo)
{
   unsigned char c;
   short val, fundo, cor;
   fundo = VID_COLOR_PRETO;
   cor = VID_COLOR_BRANCO;
	
   frame.criar_frame(x_ini, y_ini, VID_COLOR_PRETO, VID_COLOR_BRANCO, (x_fim - x_ini)+1, (y_fim - y_ini)+1, dados);
   config_cursor(1);
   posicionar_cursor(obter_x_abs(0),obter_y_abs(0));
   
   c = 0xB0;	
   for(int i = 1; i < 23; i++)
   {
	  val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) |c;
      dados[i*frame.qtd_colunas + 79] = val;
   }
   
   val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) | 0xDB;
   dados[1*frame.qtd_colunas + 79] = val;
   
   
   frame.escrever(titulo, fundo, cor, (80 - strlen(titulo))/2, 0);
   frame.escrever("ESC = SAIR  F1 = NOVO  F2 = SALVAR  F3 = RECARREGAR", fundo, cor, 20, frame.ult_linha);
   
   frame.atualizar();
   
}

void AreaDeTexto::executar_operacao(unsigned char c)
{
	if(c == ESC)
	{
	   //restaurar_area(backup); 
	}
	else if(c == ENTER)
	{
	   //divide a linha em duas	
	   texto.dividir_linha(pos_x, pos_y_texto, colunas_max);
	   pos_x = 0;
	   pos_y_tela++;
	   pos_y_texto++;
	   
	   //incrementar posi��o da primeira linha ser impressa
	   //caso o cursor esteja na �ltima linha dispon�vel
	   if(pos_y_tela + 1 > linhas_max)
	   {
			pos_y_tela--;
			prim_linha_exibida++;
	   }
	   
	   posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		
	   reescrever();	 
	}
	else if(c == BACKSPACE)
	{
	  //Se pos_y_texto � igual a zero(cursor na 1�linha do texto), ent�o obrigatoriamente pos_x deve ser maior q 0
	  if( ((pos_x == 0) && (pos_y_texto > 0)) || (pos_x > 0) )
	  {
	    //Remover caractere da posi��o x
		if(pos_x > 0)	
		{
		  pos_x--;			
		}
		else
		{	
		  
		  //Posicionar pos_x no final da linha anterior
		  unsigned int tam = texto.conteudo[pos_y_texto - 1].tamanho();
		  pos_x =  (tam == 0) ? 0 : tam - 1;
		  
		  //unir linha atual com linha anterior
		  texto.unir_linhas(pos_y_texto, colunas_max);
		  
		  pos_y_tela--;
	      pos_y_texto--;
		  
		  if(pos_y_tela < 0)
		  {
			pos_y_tela++;
			prim_linha_exibida--;
		  }
		  
		}
	
		texto.remover_char(pos_x, pos_y_texto);			
	  }
	  else if(texto[0].tamanho() > 0) // primeiro caractere do texto
	  {
		texto.remover_char(pos_x, pos_y_texto);			
	  }
	  
	  posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		
	  reescrever();	 
	  
	}
	else if(c == SETA_ESQ)
	{
	     // -Se x maior que 0
		 if(pos_x > 0)
		 {
			pos_x--;
		 }
		 else if(pos_y_texto > 0)
		 {
		    //posicionando x no final da linha anterior
		    unsigned int tam_linha_anterior = texto[pos_y_texto - 1].tamanho();
			pos_x = (tam_linha_anterior == 0) ? 0: tam_linha_anterior - 1;
			
			pos_y_texto--;
			pos_y_tela--;
			
			//caso tenha atingido o come�o da tela, rolar tela para cima
			if(pos_y_tela < 0)
			{
				pos_y_tela++;
				prim_linha_exibida--;
				reescrever();	 
			}
		 }
		 
		 posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		
	}
	else if(c == SETA_DIR)
	{
		//caso n�o enha atingido o final da linha
		if( ((pos_x + 1) <= texto[pos_y_texto].tamanho()) && ((pos_x + 1) < colunas_max) )
		{
			pos_x++;
		}
		//caso tenha atingido o final da linha
		else if(texto.conteudo.tamanho() > pos_y_texto + 1 )
		{
			pos_y_texto++;
			pos_y_tela++;
			pos_x = 0;
			
			//caso tenha atingido o final da tela
			if(pos_y_tela + 1 > linhas_max)
			{
				pos_y_tela--;
				prim_linha_exibida++;
				reescrever();
			}
		}
		posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		 
	}
	else if(c == SETA_PARA_CIMA)
	{
       if(pos_y_texto > 0)	
	   {
			pos_y_texto--;
			pos_y_tela--;
			
			if(pos_y_tela < 0)
			{
				pos_y_tela++;
				prim_linha_exibida--;
				reescrever();	 
			}
			
			unsigned int tam = texto[pos_y_texto].tamanho();
			
			if(pos_x > tam)
			   pos_x = (tam == 0 )? 0 : tam - 1;
	   }
	   posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		 
	}
	else if(c == SETA_PARA_BAIXO)
	{
	  if( (pos_y_texto+1) < texto.conteudo.tamanho())
	  {
		pos_y_texto++;
		pos_y_tela++;
		
		if((pos_y_tela+1) > linhas_max)
		{
			pos_y_tela--;
			prim_linha_exibida++;
			reescrever();	 
		}
		
		unsigned int tam = texto[pos_y_texto].tamanho();
		if(pos_x > tam)
			   pos_x = (tam == 0 )? 0 : tam - 1;
			   
		posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		 
	  }
	
	}
	else if(c >= ' ' && c <= 127)//Digitar caractere comum:
	{
	  //-Inserir caractere na posi��o especificada
	  texto.adicionar_char(c, pos_x, pos_y_texto);
	  pos_x++;
	  
	  // -Se valor de x +1 > limite de colunas,
	  if(pos_x + 1 > colunas_max)
	  {
		
		unsigned int i = pos_x-1;
		
		//procurando o come�o da �ltima palavra da linha
		while((texto.conteudo[pos_y_texto][i] != ' ') && (i > 0)) i--;
		
		if(i !=0)
		{
			texto.dividir_linha(i+1, pos_y_texto, colunas_max);
			pos_x = colunas_max - i - 1;
		}
		else
		{
			pos_x = 0;
		}
		
		pos_y_tela++;
		pos_y_texto++;
		
		// -se a linha y_texto n�o existe, adicionar ao final da lista de linhas
		if(pos_y_texto == texto.conteudo.tamanho())
		{
			texto.adicionar_linha();
		}
		
		// -Se valor de y_tela > limite de linhas na tela,
		if(pos_y_tela + 1 > linhas_max)
		{
			pos_y_tela--;
			prim_linha_exibida++;
		}
		
	  }
		
	  posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		
	  reescrever();	 
	}
}

void AreaDeTexto::executar_operacao_background(unsigned char c)
{
	if(c == ENTER)
	{
	   //divide a linha em duas	
	   texto.dividir_linha(pos_x, pos_y_texto, colunas_max);
	   pos_x = 0;
	   pos_y_tela++;
	   pos_y_texto++;
	   
	   //incrementar posi��o da primeira linha ser impressa
	   //caso o cursor esteja na �ltima linha dispon�vel
	   if(pos_y_tela + 1 > linhas_max)
	   {
			pos_y_tela--;
			prim_linha_exibida++;
	   }
	 
	}
	else if(c == BACKSPACE)
	{
	  //Se pos_y_texto � igual a zero(cursor na 1�linha do texto), ent�o obrigatoriamente pos_x deve ser maior q 0
	  if( ((pos_x == 0) && (pos_y_texto > 0)) || (pos_x > 0) )
	  {
	    //Remover caractere da posi��o x
		if(pos_x > 0)	
		{
		  texto.remover_char(pos_x, pos_y_texto);			
		  pos_x--;			
		}
		else
		{
		  //Posicionar pos_x no final da linha anterior
		  pos_x = texto.conteudo[pos_y_texto - 1].tamanho() - 1;	
		  
		  //unir linha atual com linha anterior
		  texto.unir_linhas(pos_y_texto, colunas_max);
		  
		  pos_y_tela--;
	      pos_y_texto--;
		  
		  if(pos_y_tela < 0)
		  {
			pos_y_tela++;
			prim_linha_exibida--;
		  }
		}
		
	//	posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		
	//    reescrever();	 
	  }
	  else // primeiro caractere do texto
	  {
		texto.remover_char(pos_x, pos_y_texto);			
	  }

	}
	else if(c == SETA_ESQ)
	{
	     // -Se x maior que 0
		 if(pos_x > 0)
		 {
			pos_x--;
		 }
		 else if(pos_y_texto > 0)
		 {
		    //posicionando x no final da linha anterior
		    unsigned int tam_linha_anterior = texto[pos_y_texto - 1].tamanho();
			pos_x = (tam_linha_anterior == 0) ? 0: tam_linha_anterior - 1;
			
			pos_y_texto--;
			pos_y_tela--;
			
			//caso tenha atingido o come�o da tela, rolar tela para cima
			if(pos_y_tela < 0)
			{
				pos_y_tela++;
				prim_linha_exibida--;
				reescrever();	 
			}
		 }
		 
		 //posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		
	}
	else if(c == SETA_DIR)
	{
		//caso n�o enha atingido o final da linha
		if( ((pos_x + 1) < texto[pos_y_texto].tamanho()) && ((pos_x + 1) < colunas_max) )
		{
			pos_x++;
		}
		//caso tenha atingido o final da linha
		else if(texto.conteudo.tamanho() > pos_y_texto + 1 )
		{
			pos_y_texto++;
			pos_y_tela++;
			pos_x = 0;
			
			//caso tenha atingido o final da tela
			if(pos_y_tela + 1 > linhas_max)
			{
				pos_y_tela--;
				prim_linha_exibida++;
				reescrever();
			}
		}
		//posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		 
	}
	else if(c == SETA_PARA_CIMA)
	{
       if(pos_y_texto > 0)	
	   {
			pos_y_texto--;
			pos_y_tela--;
			
			if(pos_y_tela < 0)
			{
				pos_y_tela++;
				prim_linha_exibida--;
				reescrever();	 
			}
			
			unsigned int tam = texto[pos_y_texto].tamanho();
			
			if(pos_x > tam)
			   pos_x = (tam == 0 )? 0 : tam - 1;
	   }
	   //posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		 
	}
	else if(c == SETA_PARA_BAIXO)
	{
	  if( (pos_y_texto+1) < texto.conteudo.tamanho())
	  {
		pos_y_texto++;
		pos_y_tela++;
		
		if((pos_y_tela+1) > linhas_max)
		{
			pos_y_tela--;
			prim_linha_exibida++;
			reescrever();	 
		}
		
		unsigned int tam = texto[pos_y_texto].tamanho();
		if(pos_x > tam)
			   pos_x = (tam == 0 )? 0 : tam - 1;
			   
		//posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		 
	  }
	
	}
	else if(c >= ' ' && c <= 127)
	{
	  texto.adicionar_char(c, pos_x, pos_y_texto);
	  pos_x++; 
	}
}

void AreaDeTexto::reescrever()
{
   
   Lista<unsigned char> l;
   unsigned int i;
   unsigned int j;   
	
   //varrendo todas as linhas a partir da primeira linha que dever ser escrita na tela	
   for(i = prim_linha_exibida; i < linhas_max + prim_linha_exibida; i++)
   {
		
	 //caso i seja maior que a quantidade atual de linhas, aux recebe um endere�o de uma lista posti�a vazia	
	 Lista<unsigned char> *aux = (i < texto.conteudo.tamanho()) ? &texto[i] : &l;
	 
	//caso um caractere excedente na linha, ele � removido e inserido na linha inferior
	if(aux->tamanho() > colunas_max)
	{
	     
	     for(unsigned int it =aux->tamanho()-2; it > 0; it--)
		 {
			if((*aux)[it] == ' ') break;
		 }
		 
	     //it == 0
		 if(it == 0)
		 {
		     
			 while(aux->tamanho() > colunas_max)
			 {
				
				//se i � a �ltima linha, uma nova linha precisa ser adicionada
				if( (i + 1) == texto.conteudo.tamanho())
				{
					texto.adicionar_linha();
				}
				
				//inserindo novo caractere no come�o da linha inferior
				texto.adicionar_char( (*aux)[aux->tamanho()-1], 0, i+1);
			 
				//removendo caractere da linha atual
				texto.remover_char(aux->tamanho()-1,i);
				
				aux = &texto[i];
			 }
		 }
		 else
		 {
		     unsigned int tam_ult_palavra = 0;
			 while( (aux->tamanho() > colunas_max) || (*aux)[aux->tamanho()-1] != ' ')
			 {
				
				//se i � a �ltima linha, uma nova linha precisa ser adicionada
				if( (i + 1) == texto.conteudo.tamanho())
				{
					texto.adicionar_linha();
				}
				
				//inserindo novo caractere no come�o da linha inferior
				texto.adicionar_char( (*aux)[aux->tamanho()-1], 0, i+1);
			 
				//removendo caractere da linha atual
				texto.remover_char(aux->tamanho()-1,i);
				
				aux = &texto[i];
				tam_ult_palavra++;
			 }
			 
			 if(pos_y_texto == i)
			 {
				 if( (colunas_max - pos_x) < tam_ult_palavra )
				 {
						pos_x = colunas_max - pos_x;
						pos_y_texto++;
						
						if(pos_y_tela + 1 < linhas_max)
						{
						  pos_y_tela++;
						}
						else
						{
						    prim_linha_exibida++;
							reescrever();
						}
						
						posicionar_cursor(obter_x_abs(pos_x), obter_y_abs(pos_y_tela));		 
				 }
			 }
		 }
	} 
	 
	 
	 //varrendo as colunas
     for(j = 0 ; j < colunas_max; j++)
	 {
		 //caso n�o haja caractere para imprimir, imprime espa�o vazio
		 if( (i < texto.conteudo.tamanho()) && (j < aux->tamanho()) )
	     {
			 frame.escrever((*aux)[j], VID_COLOR_PRETO, VID_COLOR_BRANCO, (j+1), i - prim_linha_exibida+1);
		 }
		 else
		 {
			 frame.escrever(' ',  VID_COLOR_PRETO, VID_COLOR_BRANCO, (j+1), i - prim_linha_exibida+1);
		 }
		 
     }//fim do for
	 
   }//fim do for
   
   
   unsigned char c;
   short val, fundo, cor;
   fundo = VID_COLOR_PRETO;
   cor = VID_COLOR_BRANCO;
   
   int pos_barra = (int)(( (float)prim_linha_exibida/texto.conteudo.tamanho()) * 22) + 1;

   c = 0xB0;
   for(int i = 1; i < 23; i++)
   {
	  val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) |c;
      dados[i*frame.qtd_colunas + 79] = val;
   }
   
   val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) | 0xDB;
   dados[pos_barra*frame.qtd_colunas + 79] = val;
   
   
   frame.atualizar();
   
}//fim da fun��o

void AreaDeTexto::salvar(char *nome)
{	
	int arq = abrir(nome,'a');
	unsigned char bloco[512];
	
	if(arq > 0)
	{
		for(int i =0; i < texto.tamanho(); i++)
		{
			Lista<unsigned char>& linha = texto[i];
			
			for(int j =0; j < linha.tamanho(); j++)
			{
				bloco[j] = linha[j];
			}
			
			bloco[linha.tamanho()] = '\n';
			
			escrever(arq, bloco, linha.tamanho() + 1);
		}
		
		fechar(arq);
	}
}

void AreaDeTexto::carregar_arquivo(char *nome)
{
	unsigned char bloco[512];
	int ret, arq;

	pos_x = 0;
	pos_y_texto = 0;
	pos_y_tela  = 0;
	prim_linha_exibida = 0; 
   
	texto.conteudo.limpar();
	texto.adicionar_linha();
	
	arq = abrir(nome, 'b');
	
	if(arq > 0)
	{
		//obtendo tamanho do arquivo
		Arq_Info info;
		obter_info_arq(arq, (char *)&info);
		
		while(true)
		{
			if(info.tamanho <= 0) break;
			
			int qtd_bytes = (info.tamanho >= 512 ? 512 : info.tamanho);
			
			ret = ler(arq, bloco, qtd_bytes);
			
			for(int i =0; i < qtd_bytes; i++)
			{
				executar_operacao_background(bloco[i]);
			}
			
			info.tamanho -= qtd_bytes;
		}	
	}
	else
	{
		arq = abrir(nome, 'A');
	}
	
	fechar(arq);
	
	reescrever();
}

void AreaDeTexto::carregar(short x_i, short y_i, short x_f, short y_f)
{
  x_ini = x_i;
  x_fim = x_f;
  y_ini = y_i;
  y_fim = y_f;
  
  colunas_max = x_fim - x_ini - 1;
  linhas_max  = y_fim - y_ini - 1;
     
  pos_x = 0;
  pos_y_texto = 0;
  pos_y_tela  = 0;
  prim_linha_exibida = 0; 
   
  texto.adicionar_linha();
}

void teste_error()
{
	asm volatile("hlt");
}


//--------------------------------------------------------------------------------------------------------
main(int argc, char ** args)
{	
	//arquivo manipulado pelo editor
	unsigned char arquivo[250], tecla;
	
	//criando area de texto
	AreaDeTexto area(0, 0, 79, 23);

	//caso um par�metro seja informado
	if(argc == 1)
	{
	   memcpy(arquivo, args[0], strlen(args[0])+1);
	}
	else //sen�o, trabalhar com arquivo normal
	{
	   memcpy(arquivo, "/texto.txt", strlen("/texto.txt")+1);
	}
	
	//inicializando
	area.inicializar(arquivo);
	
	//carregando arquivo
	area.carregar_arquivo(arquivo);
	
	
	int a = 0;
	//la�o de repeti��o do programa	
	while(TRUE)
	{
		//aguardando pressionamento da tecla
		getchar(&tecla);

		//executando opera��o sobre a �rea de texto
		area.executar_operacao(tecla);
		
		if(tecla == F1)
		{
			area.pos_x = 0;
			area.pos_y_texto = 0;
			area.pos_y_tela  = 0;
			area.prim_linha_exibida = 0; 
			area.texto.conteudo.limpar();
			area.texto.adicionar_linha();
			area.reescrever();
		}
		else if(tecla == F2)
		{
			area.salvar(arquivo);
		}
		else if(tecla == F3)
		{
			area.carregar_arquivo(arquivo);	
		}
		//fechar programa caso ESC seja pressionado
		else if(tecla == ESC)
			break;
		
	}
	
	//encerrar processo
	sair();	
}

