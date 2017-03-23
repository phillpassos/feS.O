#ifndef _GERENCIADOR
#define _GERENCIADOR
#define POS_INICIAL_KERNEL 0xC0000000
#define POS_INICIAL_KERNEL_HEAP 0xD0000000

#include "Multiboot.h"
#include "Util.h"

//Classe com os métodos de controle da memória física
class Memoria_fisica : public Recurso
{ 
  unsigned char * memoria; 
  unsigned int pos_x_ult_bloco, pos_y_ult_bloco;
  unsigned int tamanho;
  unsigned int inicio;
  unsigned int tamanho_bitmap;
  unsigned int blocos_livres;
  
  public :

  void inicializar(multiboot_info*, int); 
  void * alocar(unsigned int);
  void desalocar(void *, unsigned int);
  void obter_info(char * arq);

};

extern volatile Memoria_fisica mem_fisica;

//Classe com os métodos de controle da memória virtual
class Memoria_virtual : public Recurso
{
   unsigned int memoria_a_alocar;
   unsigned int  * pdir;
   unsigned char * ult_end_ret;
   unsigned char * topo_kernel_heap;
 
   public:	
   
   unsigned int  * pt_aux;
 
   void inicializar();
   void *obter_endereco_page_table(int pdindex);
   void *obter_end_fisico(unsigned int);
   void remover_entrada_inicial();
   void *mapear_page_table_espaco_end(void * end_fisico, void * end_virtual, unsigned int flags, unsigned int * pdir);
   void mapear_page_table(void * end_fisico, void * end_virtual, unsigned int flags);		
   void recarregar_tlb();
   void carregar_page_directory(unsigned int * pdir);
   void *sbk(unsigned int tam);
   int * obter_pdir();
};

extern volatile Memoria_virtual mem_virtual;

//funções auxiliares
void * kmalloc(unsigned int tam);
void free(void *ap);

#endif