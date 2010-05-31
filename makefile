# Makefile

#Compiler C flags
CFLAGS=-Wall
#Compiler NASM flags
NFLAGS=-t -f coff
#Compiler C
CC=gcc
#Compiler NASM
NASM= nasm
#Lib
LIBS= -llcom
#Objectos
OBJECTS= video.o main.o sprite.o kbc.o asm_kbd.o timer.o music.o ints.o isr.o queue.o rtc_asm.o rtc.o GQueue.o song.o serie.o
#NOME DO PROJECTO
LIGHT_CYCLES= lightcycles.exe

all: $(LIGHT_CYCLES)

$(LIGHT_CYCLES): $(OBJECTS) liblcom.a
	$(CC) -Wall $(OBJECTS) -L. $(LIBS) -o $(LIGHT_CYCLES)

%.o: %.c %.h
	@echo A compilar o ficheiro: $<
	@$(CC) $(CFLAGS) -c $<
	
%.o: %.asm
	@echo A compilar o ficheiro: $<
	@$(NASM) $(NFLAGS) $<
	
liblcom.a: $(OBJECTS)
	@ar -cr liblcom.a $(OBJECTS)
	
clean:
	-rm -rf *.o *.exe

rebuild: clean all