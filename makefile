# Makefile

#Compiler C flags
CFLAGS=-c -Wall
#Compiler NASM flags
NFLAGS=-t -f coff
#Compiler C
CC=gcc
#Compiler NASM
NASM= nasm
#Objectos
OBJECTS= video.o main.o sprite.o kbc.o asm_kbd.o timer.o music.o ints.o isr.o queue.o rtc_asm.o rtc.o gqueue.o song.o serie.o
#NOME DO PROJECTO
LIGHT_CYCLES= lightcycles.exe

all: $(LIGHT_CYCLES)

$(LIGHT_CYCLES): $(OBJECTS)
	$(CC) -Wall main.c -o $(LIGHT_CYCLES)

%.o: %.c %.h
	@echo A compilar o ficheiro: $<
	@$(CC) $(CFLAGS) -c $<
	
%.o: %.asm
	@echo A compilar o ficheiro: $<
	@$(NASM) $(NFLAGS) $<
	
clean:
	-rm -rf *.o *.exe

rebuild: clean all
