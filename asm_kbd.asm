CPU 686
BITS 32

global _asm_kbd

extern _teclas
extern _queuePut

section .data

_tecla dd 0

section .text

_asm_kbd:
	push eax
	push ecx
	push ebx
	push edx
	
	xor eax, eax
	IN	ax, 0x60
	
	;cli
	push eax
	push _teclas
	call _queuePut
	add esp, 8
	;sti
fim:
	mov al, 0x20
	out 0x20, al

	pop edx
	pop ebx
	pop ecx
	pop eax
	iretd
END