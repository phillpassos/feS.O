#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Stdarg.h"
#include "include/Io.h"

unsigned char buffer[512];
unsigned int pos =0;

char arquivo_out[35];
char arquivo_in[35];

volatile int pid = 0;
volatile int pid_retorno = 0;

struct Bloco_Console
{
	unsigned int codigo;
	unsigned int pid;
	unsigned int param1, param2;
	unsigned int tam;
	unsigned char in[35];
	unsigned char out[35];
}__attribute__((packed));

Bloco_Console criar_bloco(int cod, int param1, int param2, int tam)
{
	//obtendo pid do processo atual
	if(pid == 0)
	{
		pid = obter_pid(obter_pid());
	}
	
	//definindo nomes para arquivos de entrada e sa�da
	//os nomes tem a forma stdin[pid] e stdout[pid]
	if(strlen(arquivo_out) == 0)
	{
	   //convertendo o pid para string
	   char* num = itoa(pid,10);
	   
	   //criando nome stdout
	   memcpy(arquivo_out, "/dev/stdout", strlen("/dev/stdout")+1);
	   strcat(arquivo_out, num);
	  
	   //criando nome stdin
	   memcpy(arquivo_in, "/dev/stdin", strlen("/dev/stdin")+1);
	   strcat(arquivo_in, num);
	}
	
	//criando bloco
	Bloco_Console bloco; 
	bloco.codigo = cod;
	bloco.pid = pid;
	bloco.param1 = param1;
	bloco.param2 = param2;
	bloco.tam = tam;
	memcpy(bloco.in, arquivo_in, strlen(arquivo_in)+1);
    memcpy(bloco.out, arquivo_out, strlen(arquivo_out)+1);
	
		
	//retornando bloco
	return bloco;
}

void alterar_cor(int texto, int fundo)
{
   unsigned char msg[100];

   //obter pid do processo
   int pid = obter_pid();
  
   //criando dados para enviar ao servidor
   Bloco_Console b = criar_bloco(4, texto, fundo, 0);
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}

void config_cursor(int val)
{  
   //criando dados para enviar ao servidor
   unsigned char msg[100];
   
   //criando bloco
   Bloco_Console b = criar_bloco(5, val, 0, 0);
  
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}

void posicionar_cursor(int x, int y)
{
   //criando dados para enviar ao servidor
   unsigned char msg[100];
   
   //criando bloco
   Bloco_Console b = criar_bloco(1, x, y, 0);
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}

void cls()
{
   //criando dados para enviar ao servidor
   unsigned char msg[100];
   
   //criando bloco
   Bloco_Console b = criar_bloco(6, 0, 0, 0);
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}


//leitura e escrita na memoria de video
void ler_mem_video(short * dados, int x1, int y1, int x2, int y2)
{
   unsigned char msg[100];

   //unindo x1 e y1 e x2 e y2 em dois par�metros 16 bits para x e 16 para o y
   int param1 = ((x1 & 0x0000FFFF) << 16) | (y1 & 0x0000FFFF);
   int param2 = ((x2 & 0x0000FFFF) << 16) | (y2 & 0x0000FFFF);
	

   //criando dados para enviar ao servidor
   Bloco_Console b = criar_bloco(7, param1, param2, 0);
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
   
   if((x1 < 80) && (x2 < 80) && (y1 < 25) && (y2 < 25))
   {
	  int tam = (x2-x1)+1	* (y2 - y1)+1;
      int desc = abrir(arquivo_out,'A');
	  ler(desc, (char *)dados, tam*2);
	  fechar(desc);
   }
}

void esc_mem_video(short * dados, int x1, int y1, int x2, int y2)
{
   unsigned char msg[100];

   //unindo x1 e y1 e x2 e y2 em dois par�metros 16 bits para x e 16 para o y
   int param1 = ((x1 & 0x0000FFFF) << 16) | (y1 & 0x0000FFFF);
   int param2 = ((x2 & 0x0000FFFF) << 16) | (y2 & 0x0000FFFF);
	
   //criando dados para enviar ao servidor
   Bloco_Console b = criar_bloco(8, param1, param2, 0);
   
   if((x1 < 80) && (x2 < 80) && (y1 < 25) && (y2 < 25))
   {
	  int tam = (x2-x1+1)*(y2-y1+1);
      int desc = abrir(arquivo_out,'a');
	  escrever(desc, (char *)dados, tam*2);
	  fechar(desc);
   } 
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}

//impress�o e leitura de caracteres
void flush()
{
   int serv;
   
   //criando dados para enviar ao servidor
   unsigned char msg[100];

   //criando bloco 
   Bloco_Console b = criar_bloco(0, 0, 0, pos);
   
   //abrindo stdout e escrevendo dados
   int arq = _abrir(arquivo_out, 'a', &serv);
   escrever(arq, buffer, pos);
   _fechar(arq);
   
   //copiando bloco para espa�o da mensagem
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
  
   //rebobinando buffer	
   pos = 0;	
}

void puts(unsigned char * texto)
{
  //calculando tamanho da texto
  int tam = strlen(texto);
  
  //caso o texto seja maior que tamanho dispon�vel no buffer
  if((pos + tam) > 512)
  {
	 //enviar dados 
     flush();
  }
  
  if(tam < 512)
  {
	  //copiar dados para a buffer, a partir da posi��o especificada pela var�avel pos
	  memcpy(&buffer[pos], texto, tam);
	  
	  //incrementando pos com o tamanho de dados escrito no buffer
	  pos+=tam;
  }
  
}

void putch(unsigned char c)
{
   //escrevendo char no buffer	
   buffer[pos] = c;
   
   //incrementando posi��o no buffer
   pos++;
}

void scanf(const char * mascara, void * param)
{
   //va_list armazena todos os argumentos que s�o passados � fun��o
   //va_list argumentos;  
   
   //inicializa a busca pelos par�metros, come�ando a partir do par�metro mascara
  // va_start(argumentos, mascara);
   int serv;
   
   //declarando buffer
   char buf_entrada[100];
   
   //criando dados para enviar ao servidor
   unsigned char msg[100];
   
   //criando bloco
   Bloco_Console b = criar_bloco(2, 0, 0, 0);
   
   //copiando dados para a �rea da mensagem
   memcpy(msg, (unsigned char*)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
   
   //abrir stdin para ler dados
   int arq = _abrir(arquivo_in, 'A', &serv);
   int ret = ler(arq, buf_entrada, 100);
   _fechar(arq);
   
   //definindo como os dados ser�o lidos
   if(!strcmp("%d", mascara))
   {
	 //caso express�o seja %d, converter para int
	 //int * valor = va_arg(argumentos, int*);
	 int * valor = param;
	 *valor = atoi(buf_entrada);
   }
   else if(!strcmp("%f", mascara))
   {
     //caso express�o seja %f, converter para double
	 //double * valor = va_arg(argumentos, double*);
	 double * valor = param;
 
	 *valor = strtod(buf_entrada, NULL);
   }
   else if(!strcmp("%c", mascara))
   {
	 //caso express�o seja %c, converter para char
	 //char * valor = va_arg(argumentos, char*);
	 char * valor = param;
	 *valor = buf_entrada[0];
   }
   else if(!strcmp("%s", mascara))
   {
	 //caso express�o seja %s, converter para string
	 //char * valor = va_arg(argumentos, char*);
	 char * valor = param;
	 int tam = strlen(buf_entrada);
	 memcpy(valor, buf_entrada, tam);
   }
   
   //limpar lista de argumentos at� o final	  
   //va_end(argumentos);     
   
}

int getchar(char * c)
{
   //criando dados para enviar ao servidor
   unsigned char msg[100];
 
   //criando bloco 
   Bloco_Console b = criar_bloco(3, 0, 1, 0);
 
   //copiando dados para �rea da mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
    
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
   
   //armazendo msg[0] que cont�m o c�digo do caractere pressionado
   *c = msg[0];
	   
   //retornando msg[1] que cont�m 1 caso uma tecla tenha sido pressionada
   //ou 0 caso uma tecla tenha sido liberada
   return msg[1];
}

//pode ter um n�mero var�avel de par�metros
void printf(const char * text, ...)
{
   //va_list armazena todos os argumentos que s�o passados � fun��o
   va_list argumentos;  
   
   //inicializa a busca pelos par�metros, come�ando a partir do par�metro text
   //que � fixo
   va_start(argumentos, text);

   //obtendo  o tamanho do texto a ser impressos
   int tam = strlen(text);  
   
   //vari�vel para controle dos caracteres de escape
   int escape =0;
   
   //for para varrer todos os caracteres especificados em teste
   for(int i =0 ; i <tam; i++)  
   {
         //caso seja encontrado o indicador de caractere de escape 
         if(text[i] == '%') 
         {
		    // se o escape j� estiver sendo tratado
			// significa que % foi inserido duas vezes
			// ent�o o caractere de escape deve ser impresso
		    if(escape)
			{
			  escape = 0;
			  putch('%');
			}
			else //sen�o, ativar flag de caractere de escape
			{
              escape = 1;
			}
            continue;
         }
         
		 //caso a letra d seja encontrado depois de um caracte
		 //de escape, pegar o pr�ximo par�metro e exibir como integer
         if((text[i] == 'd') && (escape))
         {
            char buf[32]; 
			memset(buf, '\0', 32);
			
			//obter pr�ximo argumento como int
            const int valor = (int)va_arg(argumentos, const int);
			//int val = valor;
            char * snum = itoa2(valor, 10, buf);    
			puts(snum);
            escape = 0;            
            continue;
         }
		 
		  //caso a letra d seja encontrado depois de um caracte
		 //de escape, pegar o pr�ximo par�metro e exibir como integer
         if((text[i] == 'x') && (escape))
         {
            char buf[32]; 
			memset(buf, '\0', 32);
			
			//obter pr�ximo argumento como int
            const int valor = (int)va_arg(argumentos, const int);
			//int val = valor;
            char * snum = itoa2(valor, 16, buf);    
			puts(snum);
            escape = 0;            
            continue;
         }
		 		 
		 //caso a letra f seja encontrado depois de um caracte
		 //de escape, pegar o pr�ximo par�metro e exibir como float
		 //controle de precis�o n�o tratada
         if((text[i] == 'f') && (escape))
         {
            char * buf; 
            buf[0] = '\0';
			//obter pr�ximo argumento como double
            double valor = (double)va_arg(argumentos, const double);
            buf = ftoa(valor);    
            //my_printf("%s",buf);   
			puts(buf);
            escape = 0;            
            continue;          
         }
		 
		 
         //caso a letra c seja encontrado depois de um caracte
		 //de escape, pegar o pr�ximo par�metro e exibir como char
         if((text[i] == 'c') && (escape))
         {
		    //obter pr�ximo argumento como char
            const char valor = (char)va_arg(argumentos, const char);
            putch(valor);   
            escape = 0;           
            continue;
         }
         
         //caso a letra s seja encontrado depois de um caracte
		 //de escape, pegar o pr�ximo par�metro e exibir como string
         if((text[i] == 's') && (escape))
         {
            //obter pr�ximo argumento como ponteiro para char
            const char * valor = va_arg(argumentos, const char*);
            puts(valor);   
            escape = 0;        
            continue;
         }
          
		 //se nenhum caractere de escape foi encontrado, imprimir caractere normalmente  
		 escape = 0;
         putch(text[i]); 
    }     
    
   //enviar dados para o servidor do console
   flush();   
   
   //limpar lista de argumentos at� o final	  
   va_end(argumentos);  
}
