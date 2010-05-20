CPU 686
BITS 32

%define EOI 20h 			; define EOI como valendo 0x20
%define PIC1_CMD  0x20
%define DATA_REG  60h

; Variaveis para a Musica
global _segundos		 	; declara como acessível a módulos escritos em C
global _milis					
global _t0_isr 				; o handler também deve ser global, para ser instalado

; Variaveis para o tratamento do Scancode da Queue
global _interrupt
extern _beep
extern _putGQueue
extern _musica

section .data 				; .data é a zona de dados inicializados
	_segundos dd 0 			; declara variável nome_var com 4 bytes (dd) e inicializa-a com 0
	_milis dd 0
	contador dd 1000

section .text 				; .text é a zona de código

; Interrupcao para a Musica
_t0_isr: 					; deve ser global para ser acedida em módulos escritos em C			
	push eax
	inc dword[_milis]	
	dec dword[contador]
	jnz fim
	inc dword [_segundos]
	mov dword[contador], 1000
fim:
	mov al, EOI 			; sinaliza EOI para o PIC-1
	out PIC1_CMD, al	
	pop eax 				; todos os registos guardados
	iretd

; Interrupcao para o Scancode
_interrupt:
	push eax
	STI
    in al, DATA_REG
    push eax
    push _musica
	call _putGQueue
	add esp, 8
	cmp eax, 1				; 1 = queuePut retorna sucesso
	je fim_int
	call _beep
fim_int:
	mov al, EOI
    out PIC1_CMD, al
	pop eax
    CLI
	iretd
END							; fim de código fonte assembly
