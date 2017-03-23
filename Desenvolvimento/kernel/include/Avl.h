#ifndef _AVL
#define _AVL


#include "Memoria.h"
#include "Util.h"

//========================================================================================================================
//�rvore AVL

template <class T>
class Avl
{
   int altura; 
   unsigned int indice;

   Avl<T> * esq;	
   Avl<T> * dir;
      
   Avl<T> * rot_esq();
   Avl<T> * rot_dir();
   Avl<T> * rot_esq_dir();
   Avl<T> * rot_dir_esq();
   
   public:
   
   T * elemento;
   unsigned int qtd_itens;
   
   static Avl<T> * inserir(Avl<T> ** arv, unsigned int indice, T elemento);
   static Avl<T> * remover(Avl<T> * arv, unsigned int indice);  
   static Avl<T> * maximo(Avl<T> * arv);  
   static Avl<T> * minimo(Avl<T> * arv);  
   static Avl<T> * obter_elemento(Avl<T> * arv, unsigned int indice);  
   

   static void dump(Avl<T> * arv, int n);
};

template <class T>
static void Avl<T>::dump(Avl<T> * arv, int n)
{
	
	 if(arv != NULL )
	 {
			for(int i =0; i<= n; i++)
			{
				//my_printf("--");
			}	
			  //my_printf(" %d\n", arv->indice);	
	 
	       
			Avl<T>::dump( arv->esq,n+1);
			Avl<T>::dump( arv->dir,n+1);
	 }
	    
}

template <class T>
Avl<T> * Avl<T>::rot_esq()
{
		Avl<T> * arv; 
		arv = this->esq;
	    this->esq = arv->dir;
	    arv->dir = this;
	
	    unsigned int alt1 = MAIOR( this->esq != NULL ? this->esq->altura :  -1, this->dir != NULL ? this->dir->altura :  -1 );
		this->altura = alt1 + 1;
		unsigned int alt2 = MAIOR( arv->esq != NULL ? arv->esq->altura :  -1, this->altura );
		arv->altura  = alt2 + 1;

		return arv;
}

template <class T>
Avl<T> * Avl<T>::rot_dir()
{
		Avl<T> * arv; 
		
		arv = this->dir;
	    this->dir = arv->esq;
	    arv->esq = this;
		
		int alt1 = MAIOR( this->esq != NULL ? this->esq->altura : -1, this->dir != NULL ? this->dir->altura :  -1 );
		this->altura = alt1 + 1;
		
		int alt2 = MAIOR( arv->dir != NULL ? arv->dir->altura :  -1, this->altura);
        arv->altura  = alt2 + 1;
			
		return arv;
}

template <class T>
Avl<T> * Avl<T>::rot_esq_dir()
{
	this->dir = this->dir->rot_esq();  
	return this->rot_dir();
}

template <class T>
Avl<T> * Avl<T>::rot_dir_esq()
{
	this->esq = this->esq->rot_dir();  
	return this->rot_esq();
}

template <class T>
static Avl<T> * Avl<T>::maximo(Avl<T> * arv)
{
	if(arv == NULL)
	{
		return NULL;
	}
	else if(arv->dir == NULL)
	{
	   return arv;
	}
	else
	{
		return Avl<T>::maximo(arv->dir);
	}
}

template <class T>
static Avl<T> * Avl<T>::minimo(Avl<T> * arv)
{
	if(arv == NULL)
	{
		return NULL;
	}
	else if(arv->esq == NULL)
	{
	   return arv;
	}
	else
	{
		return Avl<T>::minimo(arv->esq);
	}
}  

template <class T>
static Avl<T> * Avl<T>::inserir(Avl<T> ** arv, unsigned int indice, T elemento)
{
	//inserindo em n�  folha
	if( (*arv) == NULL )
	{
		(*arv) = (Avl<T> *)kmalloc(sizeof(struct Avl<T>));
		(*arv)->elemento  = (T *)kmalloc(sizeof(T));
		//*(*arv)->elemento  = elemento; 
		memcpy((char *)(*arv)->elemento, (char *)&elemento, sizeof(T));
		
		(*arv)->indice    = indice;
		(*arv)->altura	  = 0;
		(*arv)->esq 	  = (*arv)->dir = NULL;
	}
	else if( indice < (*arv)->indice )
	{
		//inserindo na sub�rvore da esquerda, caso o �ndice a ser inserido
		//seja menor que o �ndice da n� ra�z da sub�rvore atual
		(*arv)->esq = Avl<T>::inserir(&(*arv)->esq, indice, elemento);
	
		//verificando se a inser��o deixou a �rvore desbalanceada
		if( ( ((*arv)->esq != NULL? (*arv)->esq->altura :  -1 ) - ( (*arv)->dir != NULL? (*arv)->dir->altura :  -1 ))  == 2 )
		{
			if( indice < (*arv)->esq->indice )
				(*arv) = (*arv)->rot_esq(); 
			else
				(*arv) = (*arv)->rot_dir_esq();
		}
	}
	else if( indice > (*arv)->indice )
	{
		//inserindo na sub�rvore da direta, caso o �ndice a ser inserido
		//seja maior que o �ndice da n� ra�z da sub�rvore atual
		(*arv)->dir = Avl<T>::inserir(&(*arv)->dir, indice, elemento);
	
		//verificando se a inser��o deixou a �rvore desbalanceada
		if( (( (*arv)->dir != NULL? (*arv)->dir->altura :  -1 ) - ( (*arv)->esq != NULL? (*arv)->esq->altura :  -1 ) ) == 2 )
		{
			if( indice > (*arv)->dir->indice )
				(*arv) = (*arv)->rot_dir();
			else
				(*arv) = (*arv)->rot_esq_dir();
		}
	}
	
	int aux = MAIOR( (*arv)->esq != NULL ? (*arv)->esq->altura :  -1, (*arv)->dir != NULL ? (*arv)->dir->altura :  -1 );	
	(*arv)->altura   = aux + 1;
	
	return (*arv);

}

template <class T>
static Avl<T> * Avl<T>::remover(Avl<T> * arv, unsigned int indice)
{
	if(arv == NULL)
	{
		return arv;
	}
	else if(arv->indice < indice)
	{
		arv->dir = Avl<T>::remover(arv->dir, indice);
			
		if( (( arv->dir != NULL? arv->dir->altura :  -1 ) - ( arv->esq != NULL? arv->esq->altura :  -1 ) ) == 2 )
		{
			if( indice > arv->dir->indice )
				arv = arv->rot_dir();
			else
				 arv = arv->rot_esq_dir();
		}
	}
	else if(arv->indice > indice)
	{
		arv->esq = Avl<T>::remover(arv->esq, indice);
		
		if( ( arv->esq != NULL? arv->esq->altura : -1 ) - ( arv->dir != NULL? arv->dir->altura : -1 )  == 2 )
		{
			if( indice < arv->esq->indice )
				arv = arv->rot_esq(); 
			else
				arv = arv->rot_dir_esq();
		}
	}
	else if( arv->indice == indice)
	{
		if( arv->esq == NULL )
		{
			Avl<T> * ptr = arv->dir;
			free(arv);
			return ptr;
		}
		else if(arv->dir == NULL )
		{
			Avl<T> * ptr = arv->esq;
			free(arv);
			return ptr;
		}
		else 
		{
			Avl<T> * min = Avl<T>::minimo(arv->dir);
			arv->indice =  min->indice;
			arv->elemento = min->elemento;
			
			arv->dir = Avl<T>::remover(arv->dir, min->indice);
			
			if( (( arv->dir != NULL? arv->dir->altura :  -1 ) - ( arv->esq != NULL? arv->esq->altura :  -1 ) ) == 2 )
			{
				if( indice > arv->dir->indice )
					arv = arv->rot_dir();
				else
					 arv = arv->rot_esq_dir();
			}
				
		}
	}
	
	unsigned int aux = MAIOR( arv->esq != NULL ? arv->esq->altura : -1, arv->dir != NULL ? arv->dir->altura : -1 );	
	arv->altura   = aux + 1;
			
	return arv;

}

template <class T>
static Avl<T> * Avl<T>::obter_elemento(Avl<T> * arv, unsigned int indice)
{
	if(arv == NULL)
	{
	    return NULL;
	}
	else if(arv->indice == indice)
	{
		return arv;
	}
	else if(indice < arv->indice)
	{
		return Avl<T>::obter_elemento(arv->esq, indice);
	}
	else
	{
		return Avl<T>::obter_elemento(arv->dir, indice);
	}
	
}


//========================================================================================================================
//Arvore
template <class T>
class Arvore :public Recurso
{

	Avl<T> * arvore;
	
	public :
	
	void adicionar(unsigned int chave, T item);
	void remover(unsigned int chave);
	T& operator[](unsigned int chave);
	void dump();
};

template <class T>
void Arvore<T>::adicionar(unsigned int chave, T item)
{
   down();
   Avl<T>::inserir(&arvore, chave, item);
   up();
}

template <class T>
void Arvore<T>::remover(unsigned int chave)
{
   down();
   arvore = Avl<T>::remover(arvore, chave);
   up();
}

template <class T>
T& Arvore<T>::operator[](unsigned int chave)
{
	down();
	Avl<T>* arv = Avl<T>::obter_elemento(arvore, chave);
	up();
	
	if(arv == NULL)
	{
	   T * ret = NULL;
	   return *ret;
	}
	else
	{
		return *arv->elemento;
	}
}

template <class T>
void Arvore<T>::dump()
{
	Avl<T>::dump(arvore, 0);
}

#endif
