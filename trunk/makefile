# Makefile

all: proj.exe

proj.exe: video.o main.o sprite.o kbc.o asm_kbd.o timer.o music.o ints.o isr.o queue.o rtc_asm.o rtc.o gqueue.o song.o serie.o
	gcc -Wall video.o main.o sprite.o kbc.o asm_kbd.o timer.o music.o ints.o isr.o queue.o rtc_asm.o rtc.o gqueue.o song.o serie.o -o proj.exe
	
video.o: video.c video.h utypes.h
	gcc -c -Wall video.c

sprite.o: sprite.c sprite.h utypes.h
	gcc -c -Wall sprite.c
	
main.o: main.c sprite.h video.h utypes.h song.h rtc.h timer.h GQueue.h queue.h kbc.h serie.h
	gcc -c -Wall main.c	
	
kbc.o: kbc.h kbc.c timer.h
	gcc -Wall  -c kbc.c -o kbc.o	
	
ints.o: ints.h ints.c 
	gcc -Wall -c ints.c -o ints.o
	
timer.o: timer.h timer.c ints.h ints.c
	gcc -Wall  -c timer.c -o timer.o	
	
music.o: music.h music.c timer.h timer.c
	gcc -Wall -c music.c -o music.o
	
asm_kbd.o: asm_kbd.asm
	nasm -t -f coff asm_kbd.asm -o asm_kbd.o
	
isr.o: isr.asm
	nasm -t -f coff isr.asm -o isr.o
	
queue.o: queue.h queue.c
	gcc -Wall  -c queue.c -o queue.o	

rtc_asm.o:rtc_asm.asm
	nasm -f coff rtc_asm.asm -o rtc_asm.o		
	
rtc.o: rtc.c rtc.h
	gcc -Wall -c rtc.c	

gqueue.o: GQueue.c GQueue.h
	gcc -Wall -c GQueue.c
	
song.o: song.c song.h
	gcc -Wall -c song.c		
		
serie.o: serie.c serie.h
	gcc -Wall -c serie.c
	
clean:
	-rm *.o *.exe
