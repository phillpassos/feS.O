#Paths
########################
PATH_ROOT   := $(shell PWD)
PATH_INC    := $(PATH_ROOT)/include
PATH_BIN    := $(PATH_ROOT)/bin
PATH_LIB    := /cygdrive/c/tcc/Desenvolvimento/lib
PROJDIRS    := $(PATH_ROOT) $(PATH_INC) $(PATH_BIN) $(PATH_LIB)
CSRCFILES   := $(shell find $(PROJDIRS) -type f -name "*.cpp") 
ASMSRCFILES := $(shell find $(PROJDIRS) -type f -name "*.asm") 
HDRFILES    := $(shell find $(PROJDIRS) -type f -name "*.h")
COBJFILES   := $(patsubst %.cpp, %.o, $(CSRCFILES)) 
ASMOBJFILES := $(patsubst %.asm, %.o, $(ASMSRCFILES)) 

#alterar nome do execut�vel aqui
EXECUTABLE  := $(PATH_BIN)/appteste.o

# FERRAMENTAS
########################
NASM    =nasm -f elf
C++     =/usr/local/cross/bin/i586-elf-c++
LD      =/usr/local/cross/bin/i586-elf-ld
RM      =rm

# FLAG DE COMPILA��O
########################
CFLAGS 	=  -Wall -w -O0 -fpermissive -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-exceptions -fno-builtin -I./include

.PHONY: all clean
	
all: $(EXECUTABLE) clean

#linkando bin�rios
$(EXECUTABLE): $(COBJFILES) $(ASMOBJFILES)
	@$(LD) -T link.ld -o $(EXECUTABLE) $(COBJFILES) $(ASMOBJFILES)	

#compilando arquivos cpp	
%.o: %.cpp
	@$(C++) $(CFLAGS)  -c -o $@ $<	

#compilando arquivos asm	
%.o: %.asm
	@$(NASM) -o $@ $<	

clean:	
	-@$(RM) $(COBJFILES) $(ASMOBJFILES)