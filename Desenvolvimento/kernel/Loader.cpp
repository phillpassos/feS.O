#include "include/Loader.h"

//Executa a análise de um imagem de arquivo e retorna uma lista de seções caso seja um arquivo ELF válido
Lista<elf_seg_t> load_elf_exec(unsigned char *imagem, unsigned int *ponto_entrada, Lista<elf_seg_t> secoes)
{
	elf_file_t * elf;
	elf_seg_t * seg;
	int i = 0, j =0;

    //obtendo cabeçalho do arquivo
	elf = (elf_file_t *)imagem;
	
	//verificando se número mágico 0x7F e assinatura ELF
	if(elf->magic[0] != 0x7F || elf->magic[1] != 'E' ||
		elf->magic[2] != 'L' || elf->magic[3] != 'F')
			return secoes;
			
	//Verificando se um arquivo ELF válido (Estaticamento Linkado)		
	if( elf->bitness   != 1 ||	// 1=32-bit, 2=64-bit 
		elf->endian    != 1 ||	// 1=little-endian, 2= big-endian
		elf->ver_1     != 1 ||	// 1=current ELF spec 
		elf->file_type != 2 ||	// 1=reloc, 2=exec, 3=DLL 
		elf->machine   != 3 ||	// 3=i386 */
		elf->ver_2     != 1)
			return secoes;
		
    //obtendo o ponto de entrada do arquivo executável
	(*ponto_entrada) = elf->entry;
	
	//buscando cabeçalho das seções do arquivo
	imagem += elf->phtab_offset;
	
	//varrendo seções do arquivo
	for(i = 0; i < elf->num_phs; i++)
	{
		//seg aponta para o o cabeçalho da seção
		seg = (elf_seg_t *)(imagem + elf->phtab_ent_size * i);
			
		//Uma seção do tipo 2=DYNAMIC e 5=SHLIB indicam que o arquivo
		//não é estaticamente linkado, e por isso não pode ser carregado
		if(seg->type == 2 || seg->type == 5)
			return secoes;
		else if(seg->type == 1) //tipo 1=LOAD (segmento que deve ser carregado na memória)
		{
		    unsigned char * pos = (unsigned char *)elf;
			pos +=seg->offset;
			
		    //adicionando nova seção à lsita
			secoes.adicionar(*seg);

		}//fim do else
		
	}//fim do for
	
	//retornando lista de seções
	return secoes;
}
