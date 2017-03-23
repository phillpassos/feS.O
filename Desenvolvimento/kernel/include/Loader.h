#ifndef _LOADER_H
#define _LOADER_H

#include "Lista.h"

//Tipos de se��es em um arquivo ELF
#define	SF_ZERO		0x10	// BSS 
#define	SF_LOAD		0x08	// carregar do arquivo 
#define	SF_READ		0x04	// readable 
#define	SF_WRITE	0x02	// writable 
#define	SF_EXEC		0x01	// execut�vel 
#define	SF_BSS		(SF_ZERO | SF_READ | SF_WRITE)

//Estrutura do cabe�alho do arquivo ELF
typedef struct
{
	unsigned char magic[4];
	unsigned char bitness;
	unsigned char endian;
	unsigned char ver_1;
	unsigned char res[9];
	unsigned short file_type;
	unsigned short machine;
	unsigned int ver_2;
	unsigned int entry;
	unsigned int phtab_offset;
	unsigned int shtab_offset;
	unsigned int flags;
	unsigned short file_hdr_size;
	unsigned short phtab_ent_size;
	unsigned short num_phs;
	unsigned short shtab_ent_size;
	unsigned short num_shs;
	unsigned short shstrtab_index;
} __attribute__((packed)) elf_file_t; 

//Estrutura do cabe�alho de uma segmento
typedef struct
{
	unsigned int type		;
	unsigned int offset		;
	unsigned int virt_adr	;
	unsigned int phys_adr	;
	unsigned int disk_size	;
	unsigned int mem_size	;
	unsigned int flags		;
	unsigned int align		;
}  __attribute__((packed)) elf_seg_t; 

//Estrutura do cabe�alho de uma se��o
typedef struct
{
  unsigned int	sh_name;		//Nome da se��o
  unsigned int	sh_type;		//Tipo da se��o
  unsigned int	sh_flags;		//Flags
  unsigned int	sh_addr;		//Endere�o virtual para execu��o
  unsigned int	sh_offset;		//Offset da se��o dentro do arquivo
  unsigned int	sh_size;		//Tamanho da se��o em bytes
  unsigned int	sh_link;		//Link para outra se��o
  unsigned int	sh_info;		//Informa��es Adicionais
  unsigned int	sh_addralign;	//Alinhamento da se��o
  unsigned int	sh_entsize;		//Tamanho do registro
} Elf32_Shdr;

//Prot�tipo da fun��o para carregar execut�vel ELF
Lista<elf_seg_t> load_elf_exec(unsigned char *imagem, unsigned int *ponto_entrada, Lista<elf_seg_t> secoes);

#endif