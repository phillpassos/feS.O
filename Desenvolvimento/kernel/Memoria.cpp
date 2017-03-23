#include "include/Memoria.h"
#include "include/Vfs.h"
#include "include/Util.h"
 
#define TAM_BLOC32 32768
#define LIM_1MB 1048576
#define NALLOC 1024

//Instância das classes de memória física e virtual
volatile Memoria_fisica mem_fisica;
volatile Memoria_virtual mem_virtual;

//========================================================================================================================
//Memória física
 /*
    mapa da memória física
	 ___________________
	|					|
	|					| Mémoria livre para o usuário
	|___________________|
	|___________________| bitmap com estados da memoria
    |___________________| byte final do kernel
    |                   |
    |______kernel_______|
 */
  
//Tamanho de uma página de memória
const unsigned int TAM_BLOC = 4096;
 
//recebe o endereço onde o kernel termina e o tamanho total da memória em bytes
void Memoria_fisica::inicializar(multiboot_info* mbd, int end_fim_kernel)
{
   unsigned long  * base;
   unsigned long  * comprimento;
   unsigned int end_final_modulos = 0;
   pos_x_ult_bloco = 0; 
   pos_y_ult_bloco = 0;
   mutex = 0;
   id_dono = -1;
   rec_num = 4;
   
   /*apenas os 4 primeiros mb de memória estão mapeados quando esse código é executado 
   dessa forma, caso o GRUB coloque a estrutura do mapa de memória em um endereço 
   após os 4 primeiros mb. O  código abaixo não funcionará */
   mbd = (multiboot_info*)((unsigned int)mbd + POS_INICIAL_KERNEL);
   multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) ((unsigned int)mbd->mmap_addr + POS_INICIAL_KERNEL);
   
   //varrendo dados obtidos pelo GRUB	
   while(mmap < (multiboot_memory_map_t *)(  ((unsigned int)mbd->mmap_addr + POS_INICIAL_KERNEL) + ((unsigned int)mbd->mmap_length) ))
   {  
		base        = (unsigned long *) ((unsigned long )&mmap->base_addr_low );
		comprimento = (unsigned long *) ((unsigned long )&mmap->length_low  );
		
		//Buscando primeiro bloco de memória livre após o 1MB de memória
		if ((mmap->type == 1)  && (*base >= LIM_1MB)) 
		{
		  break;
		}

		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int));
   }
  
   //obtendo o espaço total ocupado pelos módulos do GRUB
   multiboot_mod_list * modulos = (unsigned int)mbd->mods_addr + POS_INICIAL_KERNEL;
   
   //obtendo o endereço final ocupado pelo último módulo
   if(mbd->mods_count > 0)
   {
	 end_final_modulos = modulos[ mbd->mods_count -1].mod_end;
   }
	  
   /*Calculando o primeiro endereço disponível de forma que ele seja um
   Page boundary valido para as Page Tables (ou seja, possua os 12 bits finais iguais a 0)
   Esse é um pre-requisito para implementar a paginação na arquitetura x86*/
   inicio = (end_fim_kernel > end_final_modulos ? end_fim_kernel : end_final_modulos) + POS_INICIAL_KERNEL; 
   inicio = (inicio+TAM_BLOC);
   inicio = inicio + TAM_BLOC -(inicio%TAM_BLOC);
  
   tamanho = (*comprimento - end_fim_kernel);	 
   
   //posicionando a memoria após o kernel
   memoria = (unsigned char *) (inicio) + TAM_BLOC;
  
   //será necessário utilizar 1 bit para indicar o estado de 4 kb de memória
   long num_bits_necessario = blocos_livres = tamanho/TAM_BLOC;
  
   //o tamanho bitmap esta em bytes (numero de bits necessário dividido por 8)
   tamanho_bitmap = num_bits_necessario/8;
  
   //zerando todos os endereços
   memset(memoria, 0 ,tamanho_bitmap);
  
   //o bitmap ocupara em unidades de 4 kb; 
   if(tamanho_bitmap/TAM_BLOC <= 0)
     alocar(1);
   else	
     alocar(tamanho_bitmap/TAM_BLOC);	 
}

//Alocar memória em blocos de 4kb
void * Memoria_fisica::alocar(unsigned int tam)
{
   //down no mutex
   //down();
	
   //declarando endereço que será retornado
   void * endereco = (void *)-1;
     
   //acumular a quantidade de endereços livres consecutivos
   int qtd_livres_seguidos = 0;
   int pos_x = 0;
   int pos_y = 0;
   unsigned int valor;	 
	 
   //verificando se existe memória suficiente	 
   if(blocos_livres>= tam)
   {  
	   blocos_livres -= tam;
	  
	   //procurar por bloco com tamanho requisitado
	   for(int i =0; i < tamanho_bitmap; i++)
	   {
		   //obtendo o valor do byte na posição i
		   valor = (unsigned int) memoria[i];
		  	  
		   //varrendo cada bit do byte	  
		   for(int j =0; j < 8; j++)
		   {
			  
			  //caso o estado do bit seja 0, o frame está livre
			  if(obter_estado_bit(valor,j) == 0)
			  {
					if(qtd_livres_seguidos == 0)
					{
					  pos_x = i;
					  pos_y = j;
					}
					
					qtd_livres_seguidos++;
			  }
			  else
			  {
					qtd_livres_seguidos = 0;
			  }
			  
             //para quando encontra frames livres suficientes
			 if(qtd_livres_seguidos >= tam) break;
			 
		   }
		  
		  //para quando encontra frames livres suficientes
		  if(qtd_livres_seguidos >= tam) break;
	   }
	   
	   //alocando blocos
	   unsigned int num_blocos_alocados = 0;
	   unsigned int pos = pos_x*8 + pos_y;
	   
	   while(num_blocos_alocados < tam)
	   {
	        //marcando blocos como alocados
			setar_byte(&memoria[pos/8], pos%8,1); 
			pos++;
			num_blocos_alocados++;
	   }
	   
	   //calculando o endereço do bloco
	   endereco = (void *) (inicio + pos_x*TAM_BLOC32 + pos_y*TAM_BLOC) - POS_INICIAL_KERNEL;  
	   //pos_x_ult_bloco = pos_x;
	   //pos_y_ult_bloco = pos_y;
  }
    
  //up no mutex	 
  //up();
  
  return endereco;
}

//Desaloca memória em bloco de 4kbs
void Memoria_fisica::desalocar(void * end, unsigned int num_blocos)
{
	//down no mutex
	//down();
	
	//verificando se o endereço é diferente de 0 e é múltiplo de 4kb
    if( (end != 0) &&  ( ((int)end % 4096) == 0))
	{
	    //Calculando posição x e y no bitmap em função do endereço 
		unsigned int aux   = ((unsigned int)end) - inicio + POS_INICIAL_KERNEL;
		unsigned int pos_y = (aux%TAM_BLOC32)/TAM_BLOC;
		unsigned int pos_x = (aux - pos_y*TAM_BLOC)/TAM_BLOC32;
		unsigned int pos   = pos_x*8 + pos_y;   
		
		if(pos_x <  pos_x_ult_bloco)
		{
			//pos_x_ult_bloco = pos_x;
			//pos_y_ult_bloco = pos_y;
		}
		else if ( (pos_x == pos_x_ult_bloco) 
				&&  (pos_y < pos_y_ult_bloco))
		{
			//pos_y_ult_bloco = pos_y;
		}
		
		//varrendo a partir de pos e marcando endereços como livres
		for(unsigned int i = 0; i < num_blocos; i++)
		{
			//colocando 0 na posição respectiva do bitmap
			setar_byte(&memoria[pos/8], pos%8,0); 
			pos++;
			blocos_livres++;
		}	   
	}
	
	//up no mutex
	//up();
}


//Obtendo informações sobre a memória (Tamanho total, e espaço livre)
void Memoria_fisica::obter_info(char * arq)
{
    int descritor;
	char linha[150];

	vfs.abrir(arq, 'a', &descritor);
						
	strcat(linha, itoa(tamanho, 10));
	strcat(linha," ");
	strcat(linha, itoa(blocos_livres * TAM_BLOC, 10));
	strcat(linha, "\n");
	
	vfs.escrever(descritor, linha, strlen(linha));	
	
	vfs.fechar(descritor);
}

//========================================================================================================================
/*Memória virtual
A page table e a Page directory devem estar em endereços alinhados em 4kb. 
De forma que os 12 bits menos significativos do endereço sejam 0.
Isso é necessário pois, os 12 primeiros bits das entradas da PD e PT servem para armazenar
os flags referentes as PTs e PDs*/

unsigned int page_directory[1024]      __attribute__ ((aligned (4096)));
unsigned int primeira_page_table[1024] __attribute__ ((aligned (4096)));
unsigned int page_table_aux[1024] 	   __attribute__ ((aligned (4096)));

//Método de inicialização da memória virtual
void Memoria_virtual::inicializar()
{  
	unsigned int end = 0; 	
	memoria_a_alocar = 0;
	mutex = 0;
	id_dono = -1;
	rec_num = 5;
    void *page_directoryPtr = 0;
	void *primeira_page_tablePtr = 0;
	void *page_aux_Ptr = 0;
    
    pdir   = (unsigned int *)page_directory;
	pt_aux = (unsigned int *)page_table_aux;
	
    // Endereço físico da page directory
	page_directoryPtr =(char *)page_directory + 0x40000000;	        
	
	// Endereço físico da page table
	primeira_page_tablePtr = (char *)primeira_page_table + 0x40000000;	
   // pdirptr = page_directoryPtr;
	
    //Preenchendo a Page Diretory
    for(unsigned int i = 0; i < 1024; i++)
    {
		//atributos: nível supervisor, read/write, não presente.
		page_directory[i] = (unsigned int )0 | 2; 
		
		//Preenchendo os primeiros 4 MB da memória
		primeira_page_table[i] = end | 3; //atributos: nível supervisor, read/write, presente.
		end = end + 4096; //incrementando o endereço em 4096 (próxima página)
    }
  
	page_directory[0]  = (unsigned int)primeira_page_tablePtr; 
	page_directory[0] |= 3;//atributos: nível supervisor, read/write, presente.
	  
	//mapeando o 1/4 da parte de cima da Page Directory. Espaço compartilhado em todos os processos  
	page_directory[768]  = (unsigned int)primeira_page_tablePtr; 
	page_directory[768] |= 3;// //atributos: nível supervisor, read/write, presente.

	//Mapeando a Page Directory sobre si própria. Para poder acessar as Page Tables com endereços virtuais
	page_directory[1023]  = (unsigned int)page_directoryPtr; 
	page_directory[1023] |= 3; //atributos: nível supervisor, read/write, presente.

	//habilitando paginação. colocando endereço da Page directory no registrador cr3
	asm volatile("mov %0, %%cr3":: "b"(page_directoryPtr));

	//recarregando o valor do registrador cr0 para ativar a paginação
	unsigned int cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0)); 
	 
}

//Método executado após a remoção da GDT falsa da memórias
void Memoria_virtual::remover_entrada_inicial()
{  
	//Primeiro bloco de 4mb de memória configurado como não-presente
    //page_directory[0] = (unsigned int )0|2;  
   
	//alocando bloco de memória física
    mem_virtual.ult_end_ret = mem_fisica.alocar(1); 	  	
   
    /*Alocando espaço para as Page Tables referentes a memória do Kernel
	Como essas Page Tables são compartilhadas por todos os processos
	É mais fácil manté-las fixas, do que alocadas sob demanda */
   	for(int cont = 769; cont < 1023; cont++)
	{
		//alocando um bloco para a nova Page Table
	    unsigned int * nova_page_table = (unsigned int *)mem_fisica.alocar(1);
	    unsigned int nova_page_table_ptr = ((unsigned int)nova_page_table);
	
		//atualizando entrada na Page directory com a nova page table
		page_directory[cont] = (unsigned int) nova_page_table_ptr | 3;
	
        //obtendo o endereço virtual para acessar a page table  		
		nova_page_table = obter_endereco_page_table(cont);
	     
		//preenchendo nova page table com entradas não presentes
		for(unsigned int j = 0; j < 1024; j++)
		{
		     nova_page_table[j] = (unsigned int ) 0 | 2; //atributos: nível supervisor, read/write, não presente.
			 
		}//fim do for mais interno	
		
	}//fim do for mais externo

	//Mapeando os primeiros 4kb do HEAP do Kernel
	mapear_page_table(mem_virtual.ult_end_ret, POS_INICIAL_KERNEL_HEAP, 0x03);
    mem_virtual.ult_end_ret = mem_virtual.topo_kernel_heap = POS_INICIAL_KERNEL_HEAP;
}

/*A última entrada da Page Directory aponta para a própria Page Directory
De forma que todas as Page Tables podem ser acessadas a partir de um endereço virtual.
Esse método retorna o endereço virtual da page table, baseado em índice na page directory*/
void * Memoria_virtual::obter_endereco_page_table(int pdindex)
{
	unsigned int end  = (1023 << 22);
	end += (pdindex << 12);     
	return (void*)end;   
}

//Mapeia do endereço físico (end_fisico) no endereço virtual (end_virtual) em uma determinada page directory(pdir)
void * Memoria_virtual::mapear_page_table_espaco_end(void * end_fisico, void * end_virtual, unsigned int flags, unsigned int * pdir)
{
	
	
    void * ret = 0;
	
	//Calculando índices na page directory e page table para o endereço virtual
	//Os endereços devem estar alinhados em 4 kb
    unsigned long pdindex = (unsigned long)end_virtual >> 22;
    unsigned long ptindex = (unsigned long)end_virtual >> 12 & 0x03FF;
 
    //Verificar se a entrada na Page directory está presente (ultimo bit deve ser 1)
	if( (pdir[pdindex] & 0x01) != 0x01)
	{   
	    //alocando um bloco para a nova Page Table
	    unsigned int * nova_page_table = ret =(unsigned int *)mem_fisica.alocar(1);
	    unsigned int nova_page_table_ptr = ((unsigned int)nova_page_table);
	    
		//atualizando entrada na Page directory com a nova page table
		pdir[pdindex] = (unsigned int) nova_page_table_ptr | flags;
		
        //obtendo o endereço virtual para acessar a page table  		
		nova_page_table = obter_endereco_page_table(pdindex);
		
		//preenchendo nova page table com entradas não presentes
		for(unsigned int i = 0; i < 1024; i++)
		{
		   //atributos: nível supervisor, read/write, não presente.
		   nova_page_table[i] = (unsigned int ) 0 | 2; 
		   
		}//fim for
	    
	}//fim do if
	  
	//obtendo endereço da page table referente ao endereço virtual
	unsigned int * pt =  obter_endereco_page_table(pdindex); //(unsigned int *)( (page_directory[pdindex] & 0xFFFFF000) + POS_INICIAL_KERNEL );
	
	//Atualizando a entrada na page table
	pt[ptindex] = ((unsigned int)end_fisico) | flags; 
		
	/*retorna o endereço físico de uma Page Table caso tenha 
	sido necessário alocar uma nova senão, retorna 0*/
	return ret;
}

//Mapeia do endereço físico (end_fisico) no endereço virtual (end_virtual) na Page Directory inicial do Kernel
void Memoria_virtual::mapear_page_table(void * end_fisico, void * end_virtual, unsigned int flags)
{
	mapear_page_table_espaco_end(end_fisico, end_virtual, flags, (unsigned int *)page_directory);
	recarregar_tlb();
}

//Recarrega o TLB (Table Lookaside Buffer), buffer das estruturas de controle da memória virtal
void Memoria_virtual::recarregar_tlb()
{
	//Apenas atualiza o valor do registrador CRO
	unsigned int cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0)); 
}

//Instala a Page Directory de um novo Processo (usado pela função de troca de contexto)
void Memoria_virtual::carregar_page_directory(unsigned int * pdir)
{
	int * pdir_atual;

	//obtendo endereço do page directory atual	
	asm volatile("mov %%cr3, %0": "=b"(pdir_atual));
	
	//atualiza o registrador cr3 apenas caso um novo espaço de endereços seja selecionada
	if(pdir != pdir_atual)
	{
		//habilitando paginação. colocando endereço da Page directory no registrador cr3
		asm volatile("mov %0, %%cr3":: "b"(pdir));
		
		//recarregando Table Lookaside Buffer
		recarregar_tlb();
	}
}

//Retorna o endereço físico que será mapeado para um dado endereço virutal
void * Memoria_virtual::obter_end_fisico(unsigned int endereco_virtual)
{
	//obtendo índice na Page Directory e na Page Table	
	unsigned long pdindex = (unsigned long)endereco_virtual >> 22;
	unsigned long ptindex = (unsigned long)endereco_virtual >> 12 & 0x03FF;
	  
	//Verificando se a Page Table está presente na memória
	if((page_directory[pdindex] & 0x01) == 0x01) 
	{
		//Obtendo endereço virtual da Page Table
		unsigned int * pt =  obter_endereco_page_table(pdindex);
		
		/*Retorna endereço do frame de memória apontado pela page table, 
		somado aos 12 bits finais do endereço físico (offset dentro do frame)*/
	    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)endereco_virtual & 0xFFF));
	}
	else
	{
		//page table não presente, retorna -1
	    return -1;
	}
  
}

//Retorna o endereço do page directory padrão do kernel
int * Memoria_virtual::obter_pdir()
{
	return pdir;
}

//Aloca mais memória dinamicamente para o Kernel
void * Memoria_virtual::sbk(unsigned int tam)
{ 
    void * ret;
    void * ret_vadr = topo_kernel_heap;
	
	//Endereço a ser retornando para o usuário é o topo do HEAP atual
    ret = (void*) (((unsigned int)mem_virtual.topo_kernel_heap) + tam);
	topo_kernel_heap = ret;

	/*
	  O atributo memoria_a_alocar é o marco de quanta
	  memoria foi alocada de fato no frame de memória física atual.
	  Quando o valor de memoria_a_alocar ultrapassa o tamanho do bloco (4096 bytes)
   	  mais memória física é alocada.
	*/
	mem_virtual.memoria_a_alocar += tam;
	
	if(mem_virtual.memoria_a_alocar >=TAM_BLOC)
	{
	   //calculando quantidade de blocos que deve ser alocado	
	   unsigned int num_blocos_alocar = mem_virtual.memoria_a_alocar/TAM_BLOC + 1;
	   
	   //mapeando as páginas na memória virtual à medida que forem necessárias
	   for(unsigned int i =0; i < num_blocos_alocar; i++)
	   {
	      //alocando frame de memória física
		  ret = mem_fisica.alocar(1);
		  
		  //caso endereço retornando -1 (sem memória)
		  if (ret == -1) return ret;
		  
		  //mapeando endereço retornando no topo do HEAP
		  mem_virtual.mapear_page_table(ret, mem_virtual.ult_end_ret, 0x03);
		  
		  //incrementando valor do último endereço retornando
		  mem_virtual.ult_end_ret += TAM_BLOC;
		  
	   }//fim do for  
		
       //atualizando valor de memoria_a_alocar 		
	   mem_virtual.memoria_a_alocar = mem_virtual.memoria_a_alocar%TAM_BLOC;
	   
	}//fim do if

	
	
	return ret_vadr;
}


//========================================================================================================================
/*Implementação das funções MALLOC e FREE 
obtidas no livro The C Programming Language (K & R).
Adaptadas para funcionar junto com o Kernel.*/

typedef long Align; 

//cabeçalho do bloco
union header 
{  
	struct 
	{
	    //Próximo bloco na lista de blocos livres
		union header *ptr; 
		
		//Tamanho do próximo bloco
		unsigned int size;     
	} s;
	
	Align x; //Para fazer com que os blocos fiquem alinhados
}__attribute__((packed));

typedef union header Header;

//Criação da lista vazia 
static Header base;  

//Início da lista de blocos livres
static Header *freep = NULL; 

//Protótipos
static Header *morecore(unsigned nu);
void   free(void *ap);
void   *malloc(unsigned nbytes);

//Kmalloc: Aloca memória dinamicamente (Para uso dentro do Kernel)
void * kmalloc(unsigned int nbytes)
{
	mem_virtual.down();
	
	Header *p, *prevp;	
	unsigned nunits;
	void * end;
	
	nunits = (nbytes+sizeof(Header)-1)/sizeof(header) + 1;
	
	//Verificando se a lista de blocos livres foi inicializada
	if ((prevp = freep) == NULL)
	{ 
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}

	//Varrendo lista de blocos livres
	for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) 
	{
		//Verificando se o bloco livre é grande o sufciente
		if ( !(p->s.size < nunits)) 
		{ 
			//Verificando se o bloco tenho necessário
			if (p->s.size == nunits) 
			{
				prevp->s.ptr = p->s.ptr;
			}
			else //Caso seja maior, aloca porção final do bloco
			{ 
				p->s.size = p->s.size - nunits;
				p += p->s.size;
				p->s.size = nunits;
				
			}//fim do else
				
			freep = prevp;
			end = (void *)(p+1);
			break;
		}
		
		//Verificando se o final da lista foi atingido
		if (p == freep) 
		{
			
		   //solicita mais memória (incrementa o heap do kernel) 
		   if ((p = morecore(nunits)) == NULL)
		   {
			   end = NULL;
			   break;
		   }
		}
		
	}//fim do for
		
		
	mem_virtual.up();	
	return end;
}

//Função para liberar ou criar um novo nó na lista de blocos livres
void _free(void * ap)
{
	if((int)ap >= POS_INICIAL_KERNEL_HEAP )
	{
		Header *bp, *p;
		bp = (Header *)ap - 1; 
		
		for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
			if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
				   break; 
		
		if (bp + bp->s.size == p->s.ptr) 
		{ 
			bp->s.size += p->s.ptr->s.size;
			bp->s.ptr = p->s.ptr->s.ptr;
		} 
		else
		{
			bp->s.ptr = p->s.ptr;
		}
				
		if (p + p->s.size == bp) 
		{ 
			p->s.size += bp->s.size;
			p->s.ptr = bp->s.ptr;
		}
		else
		{
			p->s.ptr = bp;
		}

		freep = p;
	}
}

//free com lock
void free(void *ap)
{
	mem_virtual.down();
	_free(ap);
	mem_virtual.up();
}

//Função que solicita mais memória do HEAP do Kernel
static Header *morecore(unsigned nu)
{
	char *cp;
	Header *up;

	if (nu < NALLOC)
		nu = NALLOC;
		
	//chamando função SBK para solicitar mais memória	
	cp = (char *)mem_virtual.sbk(nu * sizeof(Header));
	
	//caso retorno seja -1, não há memória disponível
	if (cp == (char *) -1) 
		return NULL;
		
	up = (Header *) cp;
	up->s.size = nu;
	
	_free((void *)(up+1));
	
	return freep;
}

